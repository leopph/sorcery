#include "job_system.hpp"

#include "random.hpp"

#include <emmintrin.h>

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
        }
      }
    }, i + 1);
  }
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

  thread_local Xorshift64 xorshift{};
  auto const steal_thread_idx{xorshift() % thread_count_};

  if (steal_thread_idx == this_thread_idx) {
    _mm_pause();
    return nullptr;
  }

  if (auto const job{try_get_job_from_queue_at_idx(steal_thread_idx)}) {
    return job;
  }

  _mm_pause();
  return nullptr;
}
}
