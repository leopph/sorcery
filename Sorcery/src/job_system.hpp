#pragma once

#include "Core.hpp"
#include "observer_ptr.hpp"
#include "wsq.hpp"

#include <array>
#include <atomic>
#include <concepts>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <span>
#include <thread>
#include <type_traits>


namespace sorcery {
using JobFuncType = void(*)(void* data);
constexpr auto kMaxJobDataSize{55};


struct Job {
  JobFuncType func{nullptr};
  std::array<char, kMaxJobDataSize> data{};
  std::atomic_bool is_complete{true};
};


static_assert(sizeof(Job) == 64);

template<typename T>
concept JobArgument = sizeof(T) <= kMaxJobDataSize && std::is_copy_constructible_v<T> &&
                      std::is_trivially_destructible_v<T>;

template<typename T>
concept JobCallable = JobArgument<T> && std::invocable<T>;


class JobSystem {
public:
  LEOPPHAPI explicit JobSystem(unsigned max_thread_count = 0);

  JobSystem(JobSystem const&) = delete;
  JobSystem(JobSystem&&) = delete;

  LEOPPHAPI ~JobSystem();

  auto operator=(JobSystem const&) -> void = delete;
  auto operator=(JobSystem&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI static auto CreateJob(JobFuncType func) -> ObserverPtr<Job>;

  template<JobArgument Data>
  [[nodiscard]] static auto CreateJob(JobFuncType func, Data&& data) -> ObserverPtr<Job>;

  template<JobCallable Callable>
  [[nodiscard]] static auto CreateJob(Callable&& callable) -> ObserverPtr<Job>;

  template<JobArgument Callable, JobArgument Data> requires (
    std::invocable<Callable, Data> && !std::convertible_to<Callable, JobFuncType> && sizeof(Callable) + sizeof(Data) <=
    kMaxJobDataSize)
  [[nodiscard]] static auto CreateJob(Callable&& callable, Data&& data) -> ObserverPtr<Job>;

  template<typename T>
  [[nodiscard]] auto CreateParallelForJob(void (*func)(T& data), std::span<T> data) -> ObserverPtr<Job>;

  LEOPPHAPI auto Run(ObserverPtr<Job> job) -> void;

  LEOPPHAPI auto Wait(ObserverPtr<Job const> job) -> void;

private:
  static auto Execute(Job& job) -> void;

  [[nodiscard]] auto FindJobToExecute() -> ObserverPtr<Job>;

  unsigned thread_count_;
  unsigned worker_count_;
  std::unique_ptr<WorkStealingQueue<ObserverPtr<Job>>[]> job_queues_;
  std::unique_ptr<std::jthread[]> workers_;
  std::mutex wake_threads_mutex_;
  std::condition_variable wake_threads_cond_var_;

  constexpr static auto max_job_count_{4096};
  thread_local static std::size_t allocated_job_count_;
  thread_local static std::array<Job, max_job_count_> jobs_;
  thread_local static unsigned this_thread_idx_;
};
}


#include "job_system.inl"
