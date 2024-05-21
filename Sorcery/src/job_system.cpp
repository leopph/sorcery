#include "job_system.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

thread_local unsigned this_thread_idx;


namespace sorcery {
JobSystem::JobSystem() :
  thread_count_{std::max(std::jthread::hardware_concurrency(), 2u)} {
  auto const worker_count{thread_count_ - 1};

  job_queues_.resize(thread_count_);
  workers_.reserve(worker_count);

  for (unsigned i{0}; i < worker_count; i++) {
    workers_.emplace_back([this](std::stop_token const& stop_token, unsigned const thread_idx) {
      this_thread_idx = thread_idx;

      while (!stop_token.stop_requested()) {
        if (auto const job{FindJobToExecute()}) {
          Execute(*job);
        } else {
          std::unique_lock lock{wake_threads_mutex_};
          wake_threads_cond_var_.wait(lock);
        }
      }
    }, i + 1);
  }
}


JobSystem::~JobSystem() {
  for (auto& worker : workers_) {
    worker.request_stop();
  }

  wake_threads_cond_var_.notify_all();
}


auto JobSystem::CreateJob(JobFuncType const func) -> Job* {
  constexpr auto max_job_count{4096};
  thread_local std::size_t allocated_job_count{0};
  thread_local std::array<Job, max_job_count> jobs{};

  // TODO assert when overflowing jobs ring buffer
  // Also this method of modulus only works when max_job_count is power of two

  auto const job{&jobs[allocated_job_count++ & max_job_count - 1]};
  job->func = func;
  job->is_complete = false;
  return job;
}


auto JobSystem::Run(Job* const job) -> void {
  auto& [jobs, mutex]{job_queues_[this_thread_idx]};
  std::scoped_lock lock{*mutex};
  jobs.push(job);
  wake_threads_cond_var_.notify_all();
}


auto JobSystem::Wait(Job const* job) -> void {
  while (!job->is_complete) {
    if (auto const new_job{FindJobToExecute()}) {
      Execute(*new_job);
    }
  }
}


auto JobSystem::Execute(Job& job) -> void {
  job.func(job.data.data());
  job.is_complete = true;
}


auto JobSystem::FindJobToExecute() -> Job* {
  auto const try_get_job_from_queue_at_idx{
    [this](std::uint64_t const queue_idx) -> Job* {
      auto& [jobs, mutex]{job_queues_[queue_idx]};
      std::scoped_lock lock{*mutex};

      if (jobs.empty()) {
        return nullptr;
      }

      auto const job{jobs.front()};
      jobs.pop();
      return job;
    }
  };

  if (auto const job{try_get_job_from_queue_at_idx(this_thread_idx)}) {
    return job;
  }

  for (unsigned i{0}; i < thread_count_; i++) {
    if (i != this_thread_idx) {
      if (auto const job{try_get_job_from_queue_at_idx(i)}) {
        return job;
      }
    }
  }

  return nullptr;
}
}
