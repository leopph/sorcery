#pragma once

#include "observer_ptr.hpp"

#include <bit>
#include <memory>
#include <utility>
#include <vector>


namespace sorcery {
template<JobArgument Data>
auto JobSystem::CreateJob(JobFuncType const func, Data&& data) -> ObserverPtr<Job> {
  auto const job = CreateJob(func);
  std::construct_at(std::bit_cast<std::remove_cvref_t<Data>*>(job->data.data()), std::forward<Data>(data));
  return job;
}


template<JobCallable Callable>
auto JobSystem::CreateJob(Callable&& callable) -> ObserverPtr<Job> {
  return CreateJob([](void* const data_ptr) {
    (*std::bit_cast<Callable*>(data_ptr))();
  }, std::forward<Callable>(callable));
}


template<JobArgument Callable, JobArgument Data> requires (
  std::invocable<Callable, Data> && !std::convertible_to<Callable, JobFuncType> && sizeof(Callable) + sizeof(Data) <=
  kMaxJobDataSize)
auto JobSystem::CreateJob(Callable&& callable, Data&& data) -> ObserverPtr<Job> {
  return CreateJob([job_callable{std::forward<Callable>(callable)}, job_data{std::forward<Data>(data)}] {
    job_callable(job_data);
  });
}


template<typename T>
auto JobSystem::CreateParallelForJob(void (*func)(T& data), std::span<T> data) -> ObserverPtr<Job> {
  struct JobData {
    void (*func)(T& data);
    JobSystem* system;
    std::span<T> data;
    unsigned thread_count;
  };

  return CreateJob([](JobData const& job_data) {
    std::vector<ObserverPtr<Job>> sub_jobs;
    sub_jobs.reserve(job_data.thread_count);

    struct SubJobData {
      void (*func)(T& data);
      std::span<T> data;
    };

    auto const base_element_count = job_data.data.size() / job_data.thread_count;
    auto const element_count_remainder = job_data.data.size() % job_data.thread_count;
    std::size_t dispatched_element_count{0};

    for (auto i = 0u; i < job_data.thread_count; i++) {
      auto element_count = base_element_count;

      if (i < element_count_remainder) {
        element_count += 1;
      }

      sub_jobs.emplace_back(job_data.system->CreateJob([](void* const sub_data_ptr) {
        auto const& sub_job_data = *std::bit_cast<SubJobData*>(sub_data_ptr);

        for (auto& elem : sub_job_data.data) {
          sub_job_data.func(elem);
        }
      }, SubJobData{job_data.func, job_data.data.subspan(dispatched_element_count, element_count)}));

      job_data.system->Run(sub_jobs.back());
      dispatched_element_count += element_count;
    }

    for (auto const sub_job : sub_jobs) {
      job_data.system->Wait(sub_job);
    }
  }, JobData{func, this, data, worker_count_ + 1});
}
}
