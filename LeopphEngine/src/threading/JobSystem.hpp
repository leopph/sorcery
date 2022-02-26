#pragma once

#include "Job.hpp"
#include "../util/containers/ArrayQueue.hpp"

#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>


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
			JobSystem();

			static auto ThreadFunc(std::queue<Job>& q, std::mutex& m, std::condition_variable& cv, const std::atomic_bool& exit) -> void;

			std::queue<Job> m_Queue;
			std::mutex m_Mutex;
			std::condition_variable m_Cv;
			std::atomic_bool m_Exit{false};
			std::vector<std::thread> m_Threads;

			// One less than the number of hardware threads to account for the main thread.
			const static std::size_t NUM_THREADS;
	};
}
