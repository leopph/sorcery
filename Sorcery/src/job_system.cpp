#include "job_system.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

thread_local unsigned this_thread_idx;


namespace sorcery {
JobSystem::JobSystem() :
  thread_count_{std::max(std::jthread::hardware_concurrency(), 2u)},
  worker_count_{thread_count_ - 1} {
  job_queues_ = std::make_unique<WorkStealingQueue<Job*>[]>(thread_count_);
  workers_ = std::make_unique<std::jthread[]>(worker_count_);

  for (unsigned i{0}; i < worker_count_; i++) {
    workers_[i] = std::jthread{
      [this](std::stop_token const& stop_token, unsigned const thread_idx) {
        this_thread_idx = thread_idx;

        while (!stop_token.stop_requested()) {
          if (auto const job{FindJobToExecute()}) {
            Execute(*job);
          } else {
            std::unique_lock lock{wake_threads_mutex_};
            wake_threads_cond_var_.wait(lock);
          }
        }
      },
      i + 1
    };
  }
}


JobSystem::~JobSystem() {
  for (unsigned i{0}; i < worker_count_; i++) {
    workers_[i].request_stop();
  }

  wake_threads_cond_var_.notify_all();
}


auto JobSystem::CreateJob(JobFuncType const func) -> Job* {
  thread_local std::size_t allocated_job_count{0};
  thread_local std::array<Job, max_job_count_> jobs{};

  // This method of modulus only works when max_job_count_ is power of two
  auto const job{&jobs[allocated_job_count++ & max_job_count_ - 1]};

  if (!job->is_complete) {
    throw std::runtime_error{"Too many jobs allocated to create new!"};
  }

  job->func = func;
  job->is_complete = false;
  return job;
}


auto JobSystem::Run(Job* const job) -> void {
  auto& queue{job_queues_[this_thread_idx]};
  queue.push(job);
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
  if (auto const job{job_queues_[this_thread_idx].pop()}) {
    return *job;
  }

  for (unsigned i{0}; i < thread_count_; i++) {
    if (i != this_thread_idx) {
      if (auto const job{job_queues_[i].steal()}) {
        return *job;
      }
    }
  }

  return nullptr;
}
}
