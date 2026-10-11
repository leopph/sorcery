#pragma once

#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <thread>
#include <type_traits>

#include <wsq.hpp>

#include "Core.hpp"
#include "observer_ptr.hpp"


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
concept JobArgument = sizeof(T) <= kMaxJobDataSize &&
                      std::is_copy_constructible_v<T> &&
                      std::is_trivially_destructible_v<T>;

template<typename T>
concept JobCallable = JobArgument<T> && std::invocable<T>;


class JobSystem {
public:
  SORCERYAPI
  explicit JobSystem(std::uint32_t max_thread_count = 0);

  JobSystem(JobSystem const&) = delete;
  JobSystem(JobSystem&&) = delete;

  SORCERYAPI
  ~JobSystem();

  auto operator=(JobSystem const&) -> void = delete;
  auto operator=(JobSystem&&) -> void = delete;

  [[nodiscard]] SORCERYAPI static
  auto CreateJob(JobFuncType func) -> ObserverPtr<Job>;

  template<JobArgument Data>
  [[nodiscard]] static
  auto CreateJob(JobFuncType func, Data&& data) -> ObserverPtr<Job>;

  template<JobCallable Callable>
  [[nodiscard]] static
  auto CreateJob(Callable&& callable) -> ObserverPtr<Job>;

  template<JobArgument Callable, JobArgument Data>
    requires (std::invocable<Callable, Data> &&
              !std::convertible_to<Callable, JobFuncType> &&
              sizeof(Callable) + sizeof(Data) <= kMaxJobDataSize)
  [[nodiscard]] static
  auto CreateJob(Callable&& callable, Data&& data) -> ObserverPtr<Job>;

  template<typename T>
  [[nodiscard]]
  auto CreateParallelForJob(void (*func)(T& data), std::span<T> data) -> ObserverPtr<Job>;

  SORCERYAPI
  auto Run(ObserverPtr<Job> job) -> void;

  SORCERYAPI
  auto Wait(ObserverPtr<Job const> job) -> void;

private:
  auto WorkerLoop(std::stop_token const& stop_token, std::uint32_t thread_idx) -> void;

  auto Execute(Job& job) -> void;

  [[nodiscard]]
  auto TryFindJob() -> ObserverPtr<Job>;

  auto NotifyWork() -> void;
  auto NotifyCompletion() -> void;

  std::uint32_t thread_count_;
  std::uint32_t worker_count_;
  std::atomic_uint64_t work_epoch_{0};
  std::atomic_uint64_t progress_epoch_{0};
  std::unique_ptr<WorkStealingQueue<ObserverPtr<Job>>[]> job_queues_;
  std::unique_ptr<std::jthread[]> workers_;

  constexpr static auto kMaxJobCount{4096};
  constexpr static auto kInvalidThreadIdx{std::numeric_limits<std::uint32_t>::max()};
  thread_local static std::size_t allocated_job_count_;
  thread_local static std::array<Job, kMaxJobCount> jobs_;
  thread_local static std::uint32_t this_thread_idx_;
};
}


#include "job_system.inl"
