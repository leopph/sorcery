#pragma once

#include "Core.hpp"
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
  LEOPPHAPI JobSystem();

  JobSystem(JobSystem const&) = delete;
  JobSystem(JobSystem&&) = delete;

  LEOPPHAPI ~JobSystem();

  auto operator=(JobSystem const&) -> void = delete;
  auto operator=(JobSystem&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI static auto CreateJob(JobFuncType func) -> Job*;
  template<JobArgument T>
  [[nodiscard]] static auto CreateJob(JobFuncType func, T const& data) -> Job*;
  template<JobCallable Callable>
  [[nodiscard]] static auto CreateJob(Callable&& callable) -> Job*;

  template<typename T>
  [[nodiscard]] auto CreateParallelForJob(void (*func)(T& data), std::span<T> data) -> Job*;

  LEOPPHAPI auto Run(Job* job) -> void;

  LEOPPHAPI auto Wait(Job const* job) -> void;

private:
  constexpr static auto max_job_count_{4096};

  static auto Execute(Job& job) -> void;

  [[nodiscard]] auto FindJobToExecute() -> Job*;

  unsigned thread_count_;
  unsigned worker_count_;
  std::unique_ptr<WorkStealingQueue<Job*>[]> job_queues_;
  std::unique_ptr<std::jthread[]> workers_;
  std::mutex wake_threads_mutex_;
  std::condition_variable wake_threads_cond_var_;
};
}


#include "job_system.inl"
