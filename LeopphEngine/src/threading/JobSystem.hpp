#pragma once

#include "Job.hpp"
#include "SpinLock.hpp"
#include "Worker.hpp"
#include "../util/containers/ArrayQueue.hpp"

#include <atomic>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <unordered_map>
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
			// At least this number of workers will be created.
			constexpr static std::size_t MIN_WORKER_COUNT{2};

			// One less than the number of hardware threads to account for the main thread.
			const static std::size_t NUM_WORKERS;

			std::unordered_map<Job::Label, ArrayQueue<Job>> m_Queues;
			SpinLock m_Lock;
			std::atomic_bool m_Exit{false};
			std::vector<Worker> m_Workers;

			JobSystem();

			static auto WorkerFunc(std::add_lvalue_reference_t<decltype(m_Queues)::mapped_type> q,
			                       std::add_lvalue_reference_t<decltype(m_Lock)> lock,
			                       const std::atomic_bool& exit) -> void;
	};
}
