#include "JobSystem.hpp"

#include "Windows.h"

#include <utility>


namespace leopph::internal
{
	JobSystem::JobSystem()
	{
		m_Threads.reserve(NUM_THREADS);
		for (auto i = 0ull; i < NUM_THREADS; ++i)
		{
			m_Threads.emplace_back(ThreadFunc, std::ref(m_Queue), std::ref(m_Mutex), std::ref(m_Cv), std::ref(m_Exit));
		}
	}


	JobSystem::~JobSystem()
	{
		m_Exit = true;
		m_Cv.notify_all();

		for (auto& thread : m_Threads)
		{
			thread.join();
		}
	}


	auto JobSystem::Create() -> std::unique_ptr<JobSystem>
	{
		return std::unique_ptr<JobSystem>{new JobSystem{}};
	}


	auto JobSystem::Execute(Job job) -> Job::FutureType
	{
		Job::FutureType ret;
		{
			std::unique_lock lk{m_Mutex};
			ret = job.Future();
			m_Queue.push(std::move(job));
		}
		m_Cv.notify_one();
		return ret;
	}


	auto JobSystem::ThreadFunc(std::queue<Job>& q, std::mutex& m, std::condition_variable& cv, const std::atomic_bool& exit) -> void
	{
		Job job;

		while (true)
		{
			{
				std::unique_lock lk{m};
				cv.wait(lk, [&]
				{
					return !q.empty() || exit;
				});

				if (exit)
				{
					break;
				}

				job = std::move(q.front());
				q.pop();
			}
			job();
		}
	}


	const std::size_t JobSystem::NUM_THREADS
	{
		[]
		{
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			return sysInfo.dwNumberOfProcessors - 1;
		}()
	};
}
