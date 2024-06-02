#pragma once

#include <bit>
#include <memory>
#include <utility>
#include <vector>


namespace sorcery {
template<JobArgument Data>
auto JobSystem::CreateJob(JobFuncType const func, Data&& data) -> Job* {
  auto const job{CreateJob(func)};
  std::construct_at(std::bit_cast<std::remove_cvref_t<Data>*>(job->data.data()), std::forward<Data>(data));
  return job;
}


template<JobCallable Callable>
auto JobSystem::CreateJob(Callable&& callable) -> Job* {
  return CreateJob([](void* const data_ptr) {
    (*std::bit_cast<Callable*>(data_ptr))();
  }, std::forward<Callable>(callable));
}


template<typename T>
auto JobSystem::CreateParallelForJob(void (*func)(T& data), std::span<T> data) -> Job* {
  struct JobData {
    void (*func)(T& data);
    JobSystem* system;
    std::span<T> data;
    unsigned thread_count;
  };

  return CreateJob([](void const* const data_ptr) {
    auto const& job_data{*std::bit_cast<JobData*>(data_ptr)};
    auto const elem_count_per_job{job_data.data.size() / job_data.thread_count};

    std::vector<Job*> sub_jobs;
    sub_jobs.reserve(elem_count_per_job);

    struct SubJobData {
      void (*func)(T& data);
      std::span<T> data;
    };

    for (unsigned i{0}; i < job_data.thread_count; i++) {
      sub_jobs.emplace_back(job_data.system->CreateJob([](void* const sub_data_ptr) {
        auto const& sub_job_data{*std::bit_cast<SubJobData*>(sub_data_ptr)};

        for (auto& elem : sub_job_data.data) {
          sub_job_data.func(elem);
        }
      }, SubJobData{job_data.func, job_data.data.subspan(i * elem_count_per_job, elem_count_per_job)}));

      job_data.system->Run(sub_jobs.back());
    }

    for (auto const* const sub_job : sub_jobs) {
      job_data.system->Wait(sub_job);
    }
  }, JobData{func, this, data, worker_count_ + 1});
}
}
