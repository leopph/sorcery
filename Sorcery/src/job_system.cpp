#include "job_system.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <exception>
#include <iostream>
#include <stdexcept>


namespace sorcery {
JobSystem::JobSystem(std::uint32_t const max_thread_count) :
  thread_count_{
    [max_thread_count] {
      auto const preferred_thread_count = std::max(std::jthread::hardware_concurrency(), 2u);
      return max_thread_count == 0 ? preferred_thread_count : std::min(preferred_thread_count, max_thread_count);
    }()
  } {
  this_thread_idx_ = 0; // main thread is thread 0
  job_queues_ = std::make_unique<WorkStealingQueue<ObserverPtr<Job>>[]>(thread_count_);
  workers_ = std::make_unique<std::jthread[]>(worker_count_);

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
  // kJobSlotsPerThread must be a power of two.
  assert(std::has_single_bit(kJobSlotsPerThread));
  ObserverPtr const job{&job_slots_[next_job_slot_++ & kJobSlotsPerThread - 1]};

  if (!job->is_complete) {
    throw std::runtime_error{"Failed to allocate job: too many concurrent jobs!"};
  }

  job->func = func;
  job->is_complete = false;
  return job;
}


auto JobSystem::Submit(ObserverPtr<Job> const job) -> void {
  assert(this_thread_idx_ != kInvalidThreadIdx && this_thread_idx_ < thread_count_ &&
    "JobSystem used from unknown thread!");
  job_queues_[this_thread_idx_].push(job);
  NotifyWork();
}


auto JobSystem::Wait(ObserverPtr<Job const> const job) -> void {
  while (!job->is_complete.load(std::memory_order_acquire)) {
    auto const epoch = wait_epoch_.load(std::memory_order_acquire);

    if (auto const other = TryTakeJob()) {
      Execute(*other);
      continue;
    }

    if (job->is_complete.load(std::memory_order_acquire)) {
      break;
    }

    wait_epoch_.wait(epoch, std::memory_order_acquire);
  }
}


auto JobSystem::WorkerLoop(std::stop_token const& stop_token, std::uint32_t const thread_idx) -> void {
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


auto JobSystem::Execute(Job& job) -> void {
  try {
    job.func(job.payload.data());
  } catch (std::exception const& ex) {
    // TODO replace with more robust handling
    std::cerr << "Exception while executing job: " << ex.what() << '\n';
  } catch (...) {
    std::cerr << "Unknown exception while executing job.\n";
  }

  job.is_complete.store(true, std::memory_order_release);
  NotifyCompletion();
}


auto JobSystem::TryTakeJob() -> ObserverPtr<Job> {
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


auto JobSystem::NotifyWork() -> void {
  work_epoch_.fetch_add(1, std::memory_order_release);
  work_epoch_.notify_one();

  wait_epoch_.fetch_add(1, std::memory_order_release);
  wait_epoch_.notify_one();
}


auto JobSystem::NotifyCompletion() -> void {
  wait_epoch_.fetch_add(1, std::memory_order_release);
  wait_epoch_.notify_all();
}


thread_local std::size_t JobSystem::next_job_slot_{0};
thread_local std::array<Job, JobSystem::kJobSlotsPerThread> JobSystem::job_slots_{};
thread_local std::uint32_t JobSystem::this_thread_idx_{kInvalidThreadIdx};
}
