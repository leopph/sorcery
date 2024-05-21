#pragma once

#include "Core.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <span>
#include <thread>
#include <type_traits>
#include <vector>


namespace sorcery {
using JobFuncType = void(*)(void const* data);
constexpr auto kMaxJobDataSize{55};


struct Job {
  JobFuncType func;
  std::array<char, kMaxJobDataSize> data;
  std::atomic_bool is_complete;
};


static_assert(sizeof(Job) == 64);


class JobSystem {
public:
  LEOPPHAPI JobSystem();

  JobSystem(JobSystem const&) = delete;
  JobSystem(JobSystem&&) = delete;

  LEOPPHAPI ~JobSystem();

  auto operator=(JobSystem const&) -> void = delete;
  auto operator=(JobSystem&&) -> void = delete;

  template<typename T> requires(sizeof(T) <= kMaxJobDataSize && std::is_trivially_copy_constructible_v<T> &&
                                std::is_trivially_destructible_v<T>)
  [[nodiscard]] static auto CreateJob(JobFuncType func, T const& data) -> Job*;
  [[nodiscard]] LEOPPHAPI static auto CreateJob(JobFuncType func) -> Job*;

  template<typename T>
  [[nodiscard]] auto CreateParallelForJob(void (*func)(T& data), std::span<T> data) -> Job*;

  LEOPPHAPI auto Run(Job* job) -> void;

  LEOPPHAPI auto Wait(Job const* job) -> void;

private:
  struct JobQueue {
    std::queue<Job*> jobs;
    std::unique_ptr<std::mutex> mutex{std::make_unique<std::mutex>()};
  };


  static auto Execute(Job& job) -> void;

  [[nodiscard]] auto FindJobToExecute() -> Job*;

  unsigned thread_count_;
  std::vector<JobQueue> job_queues_;
  std::vector<std::jthread> workers_;
  std::mutex wake_threads_mutex_;
  std::condition_variable wake_threads_cond_var_;
};
}


#include "job_system.inl"
