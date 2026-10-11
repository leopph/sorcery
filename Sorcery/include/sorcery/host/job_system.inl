#pragma once

#include <bit>
#include <memory>
#include <utility>
#include <vector>

#include <sorcery/observer_ptr.hpp>


namespace sorcery {
template<JobPayload Payload>
auto JobSystem::CreateJob(JobFuncType const func, Payload&& payload) -> ObserverPtr<Job> {
  auto const job = CreateJob(func);
  std::construct_at(std::bit_cast<std::remove_cvref_t<Payload>*>(job->payload.data()), std::forward<Payload>(payload));
  return job;
}


template<JobCallable Callable>
auto JobSystem::CreateJob(Callable&& callable) -> ObserverPtr<Job> {
  return CreateJob([](void* const data_ptr) {
    (*std::bit_cast<Callable*>(data_ptr))();
  }, std::forward<Callable>(callable));
}


template<JobPayload Callable, JobPayload Payload> requires (
  std::invocable<Callable, Payload> && !std::convertible_to<Callable, JobFuncType> && sizeof(Callable) + sizeof(Payload)
  <=
  kJobPayloadCapacity)
auto JobSystem::CreateJob(Callable&& callable, Payload&& payload) -> ObserverPtr<Job> {
  return CreateJob([job_callable{std::forward<Callable>(callable)}, job_data{std::forward<Payload>(payload)}] {
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

      job_data.system->Submit(sub_jobs.back());
      dispatched_element_count += element_count;
    }

    for (auto const sub_job : sub_jobs) {
      job_data.system->Wait(sub_job);
    }
  }, JobData{func, this, data, worker_count_ + 1});
}
}
