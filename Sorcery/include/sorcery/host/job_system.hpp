#pragma once

#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <span>
#include <thread>
#include <type_traits>

#include <wsq.hpp>

#include "sorcery/Core.hpp"
#include "sorcery/observer_ptr.hpp"


namespace sorcery {
using JobFuncType = void(*)(void* data);
constexpr auto kJobPayloadCapacity{55};


enum class JobState : std::uint8_t {
  kCreated,
  kQueued,
  kRunning,
  kCompleted,
  kFailed
};


[[nodiscard]] constexpr
auto IsFinished(JobState const state) noexcept { return state >= JobState::kCompleted; }


struct Job {
  JobFuncType func{nullptr};
  alignas(std::exception_ptr) std::array<char, kJobPayloadCapacity> payload{};
  std::atomic<JobState> state{JobState::kCompleted};
};


static_assert(sizeof(Job) == 64);
static_assert(sizeof(std::exception_ptr) <= kJobPayloadCapacity);

template<typename T>
concept JobPayload = sizeof(T) <= kJobPayloadCapacity &&
                     std::is_copy_constructible_v<T> &&
                     std::is_trivially_destructible_v<T>;

template<typename T>
concept JobCallable = JobPayload<T> && std::invocable<T>;


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

  template<JobPayload Payload>
  [[nodiscard]] static
  auto CreateJob(JobFuncType func, Payload&& payload) -> ObserverPtr<Job>;

  template<JobCallable Callable>
  [[nodiscard]] static
  auto CreateJob(Callable&& callable) -> ObserverPtr<Job>;

  template<JobPayload Callable, JobPayload Payload>
    requires (std::invocable<Callable, Payload> &&
              !std::convertible_to<Callable, JobFuncType> &&
              sizeof(Callable) + sizeof(Payload) <= kJobPayloadCapacity)
  [[nodiscard]] static
  auto CreateJob(Callable&& callable, Payload&& payload) -> ObserverPtr<Job>;

  template<typename T>
  [[nodiscard]]
  auto CreateParallelForJob(void (*func)(T& data), std::span<T> data) -> ObserverPtr<Job>;

  SORCERYAPI
  auto Submit(ObserverPtr<Job> job) -> void;

  // Waits until `job` has finished. Rethrows the exception on failure.
  SORCERYAPI
  auto Wait(ObserverPtr<Job const> job) -> void;

  // Waits until `job` has finished. Returns the exception on failure, null otherwise.
  [[nodiscard]] SORCERYAPI
  auto TryWait(ObserverPtr<Job const> job) noexcept -> std::exception_ptr;

  // Waits until every job in `jobs` has finished, even if some fail, then rethrows the exception
  // of the first failed job in span order. Exceptions of later failed jobs are discarded.
  SORCERYAPI
  auto WaitAll(std::span<ObserverPtr<Job> const> jobs) -> void;

  // Waits until every job in `jobs` has finished, even if some fail, then returns the exception
  // of the first failed job (null if none failed) in span order. Exceptions of later failed jobs are discarded.
  [[nodiscard]] SORCERYAPI
  auto TryWaitAll(std::span<ObserverPtr<Job> const> jobs) noexcept -> std::exception_ptr;

private:
  auto WorkerLoop(std::stop_token const& stop_token, std::uint32_t thread_idx) noexcept -> void;

  auto Execute(Job& job) noexcept -> void;

  [[nodiscard]]
  auto TryTakeJob() noexcept -> ObserverPtr<Job>;

  auto NotifyWork() noexcept -> void;
  auto NotifyCompletion() noexcept -> void;

  std::uint32_t thread_count_;
  std::uint32_t worker_count_{thread_count_ - 1};
  std::atomic_uint64_t work_epoch_{0};
  std::atomic_uint64_t wait_epoch_{0};
  std::unique_ptr<WorkStealingQueue<ObserverPtr<Job>>[]> job_queues_;
  std::unique_ptr<std::jthread[]> workers_;

  constexpr static std::uint32_t kJobSlotsPerThread{4096};
  constexpr static std::uint32_t kInvalidThreadIdx{std::numeric_limits<std::uint32_t>::max()};
  thread_local static std::size_t next_job_slot_;
  thread_local static std::array<Job, kJobSlotsPerThread> job_slots_;
  thread_local static std::uint32_t this_thread_idx_;
};
}


#include <sorcery/host/job_system.inl>
