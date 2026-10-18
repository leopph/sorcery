#include "sorcery/host/job_system.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>


namespace sorcery {
namespace {
[[nodiscard]] auto ErrorSlot(Job& job) noexcept -> std::exception_ptr* {
  return std::launder(reinterpret_cast<std::exception_ptr*>(job.payload.data()));
}


[[nodiscard]] auto ErrorSlot(Job const& job) noexcept -> std::exception_ptr const* {
  return std::launder(reinterpret_cast<std::exception_ptr const*>(job.payload.data()));
}
}


JobSystem::JobSystem(std::uint32_t const max_thread_count) :
  thread_count_{
    [max_thread_count] {
      auto const preferred_thread_count = std::max(std::jthread::hardware_concurrency(), 2u);
      return max_thread_count == 0 ? preferred_thread_count : std::min(preferred_thread_count, max_thread_count);
    }()
  },
  job_storage_{std::make_unique<ThreadJobStorage[]>(thread_count_)},
  job_queues_{std::make_unique<WorkStealingQueue<ObserverPtr<Job>>[]>(thread_count_)},
  workers_{std::make_unique<std::jthread[]>(worker_count_)} {
  this_thread_idx_ = 0; // main thread is thread 0

  for (auto i = 0u; i < worker_count_; i++) {
    workers_[i] = std::jthread{
      [this](std::stop_token const& stop_token, std::uint32_t const thread_idx) {
        WorkerLoop(stop_token, thread_idx);
      },
      i + 1 // workers are thread 1...N
    };
  }
}


JobSystem::~JobSystem() {
  for (unsigned i{0}; i < worker_count_; i++) {
    workers_[i].request_stop();
  }

  work_epoch_.fetch_add(1, std::memory_order_release);
  work_epoch_.notify_all();
}


auto JobSystem::CreateJob(JobFuncType const func) -> ObserverPtr<Job> {
  static_assert(std::has_single_bit(kJobSlotsPerThread) && "Job slot count must be a power of two");
  assert(this_thread_idx_ != kInvalidThreadIdx && this_thread_idx_ < thread_count_ &&
    "JobSystem used from unknown thread!");

  auto& job_storage = job_storage_[this_thread_idx_];
  ObserverPtr const job{&job_storage.slots[job_storage.next_slot++ & kJobSlotsPerThread - 1]};

  auto const job_state = job->state.load(std::memory_order_acquire);

  if (!IsFinished(job_state)) {
    throw std::runtime_error{"Failed to allocate job: too many concurrent jobs!"};
  }

  if (job_state == JobState::kFailed) {
    std::destroy_at(ErrorSlot(*job));
  }

  job->func = func;
  job->state.store(JobState::kCreated, std::memory_order_relaxed);
  return job;
}


auto JobSystem::Submit(ObserverPtr<Job> const job) -> void {
  assert(this_thread_idx_ != kInvalidThreadIdx && this_thread_idx_ < thread_count_ &&
    "JobSystem used from unknown thread!");
  [[maybe_unused]] auto const old_state = job->state.exchange(JobState::kQueued, std::memory_order_relaxed);
  assert(old_state == JobState::kCreated);
  job_queues_[this_thread_idx_].push(job);
  NotifyWork();
}


auto JobSystem::Wait(ObserverPtr<Job const> const job) -> void {
  if (auto const ex = TryWait(job)) {
    std::rethrow_exception(ex);
  }
}


auto JobSystem::TryWait(ObserverPtr<Job const> const job) noexcept -> std::exception_ptr {
  auto const is_finished = [&] { return IsFinished(job->state.load(std::memory_order_acquire)); };

  while (!is_finished()) {
    auto const epoch = wait_epoch_.load(std::memory_order_acquire);

    if (auto const other = TryTakeJob()) {
      Execute(*other);
      continue;
    }

    if (is_finished()) {
      break;
    }

    wait_epoch_.wait(epoch, std::memory_order_acquire);
  }

  if (job->state.load(std::memory_order_acquire) == JobState::kFailed) {
    return *ErrorSlot(*job);
  }

  return nullptr;
}


auto JobSystem::WaitAll(std::span<ObserverPtr<Job> const> const jobs) -> void {
  if (auto const ex = TryWaitAll(jobs)) {
    std::rethrow_exception(ex);
  }
}


auto JobSystem::TryWaitAll(std::span<ObserverPtr<Job> const> const jobs) noexcept -> std::exception_ptr {
  std::exception_ptr first;

  for (auto const job : jobs) {
    if (auto ex = TryWait(job); ex && !first) {
      first = std::move(ex);
    }
  }

  return first;
}


auto JobSystem::WorkerLoop(std::stop_token const& stop_token, std::uint32_t const thread_idx) noexcept -> void {
  this_thread_idx_ = thread_idx;

  while (true) {
    auto const epoch = work_epoch_.load(std::memory_order_acquire);

    if (auto const job = TryTakeJob()) {
      Execute(*job);
      continue;
    }

    if (stop_token.stop_requested()) {
      break;
    }

    work_epoch_.wait(epoch, std::memory_order_acquire);
  }
}


auto JobSystem::Execute(Job& job) noexcept -> void {
  auto final_state = JobState::kCompleted;
  job.state.store(JobState::kRunning, std::memory_order_relaxed);

  try {
    job.func(job.payload.data());
  } catch (...) {
    std::construct_at(ErrorSlot(job), std::current_exception());
    final_state = JobState::kFailed;
  }

  job.state.store(final_state, std::memory_order_release);
  NotifyCompletion();
}


auto JobSystem::TryTakeJob() noexcept -> ObserverPtr<Job> {
  assert(this_thread_idx_ != kInvalidThreadIdx && this_thread_idx_ < thread_count_ &&
    "JobSystem used from unknown thread!");

  if (auto const job = job_queues_[this_thread_idx_].pop()) {
    return *job;
  }

  for (unsigned i{0}; i < thread_count_; i++) {
    if (i != this_thread_idx_) {
      if (auto const job = job_queues_[i].steal()) {
        return *job;
      }
    }
  }

  return nullptr;
}


auto JobSystem::NotifyWork() noexcept -> void {
  work_epoch_.fetch_add(1, std::memory_order_release);
  work_epoch_.notify_one();

  wait_epoch_.fetch_add(1, std::memory_order_release);
  wait_epoch_.notify_one();
}


auto JobSystem::NotifyCompletion() noexcept -> void {
  wait_epoch_.fetch_add(1, std::memory_order_release);
  wait_epoch_.notify_all();
}


JobSystem::ThreadJobStorage::~ThreadJobStorage() {
  for (auto& job : slots) {
    if (job.state.load(std::memory_order_acquire) == JobState::kFailed) {
      std::destroy_at(ErrorSlot(job));
    }
  }
}


thread_local std::uint32_t JobSystem::this_thread_idx_{kInvalidThreadIdx};
}
