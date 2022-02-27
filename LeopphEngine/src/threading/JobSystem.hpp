#pragma once

#include "Job.hpp"
#include "SpinLock.hpp"
#include "../util/containers/ArrayQueue.hpp"

#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>


namespace leopph::internal
{
	class JobSystem
	{
		public:
			[[nodiscard]] static auto Create() -> std::unique_ptr<JobSystem>;

			auto Execute(Job job) -> Job::FutureType;

			JobSystem(const JobSystem& other) = delete;
			auto operator=(const JobSystem& other) -> JobSystem& = delete;

			JobSystem(JobSystem&& other) noexcept = delete;
			auto operator=(JobSystem&& other) noexcept -> JobSystem& = delete;

			~JobSystem();

		private:
			// One less than the number of hardware threads to account for the main thread.
			const static std::size_t NUM_THREADS;

			ArrayQueue<Job> m_Queue;
			SpinLock m_Lock;
			std::atomic_bool m_Exit{false};
			std::vector<std::thread> m_Threads;

			JobSystem();

			static auto ThreadFunc(std::add_lvalue_reference_t<decltype(m_Queue)> q,
			                       std::add_lvalue_reference_t<decltype(m_Lock)> lock,
			                       const std::atomic_bool& exit) -> void;
	};
}
