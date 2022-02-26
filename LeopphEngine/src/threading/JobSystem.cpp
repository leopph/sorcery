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


	auto JobSystem::Execute(Job job) -> void
	{
		{
			std::unique_lock lk{m_Mutex};
			m_Queue.Push(std::move(job));
		}
		m_Cv.notify_one();
	}


	auto JobSystem::ThreadFunc(ArrayQueue<Job>& q, std::mutex& m, std::condition_variable& cv, const std::atomic_bool& exit) -> void
	{
		Job job;

		while (true)
		{
			{
				std::unique_lock lk{m};
				cv.wait(lk, [&]
				{
					return !q.Empty() || exit;
				});

				if (exit)
				{
					break;
				}

				job = {q.Pop()};
			}
			job();
		}
	}


	const std::size_t JobSystem::NUM_THREADS
	{
		[]()
		{
			SYSTEM_INFO sys_info;
			GetSystemInfo(&sys_info);
			return sys_info.dwNumberOfProcessors - 1;
		}()
	};
}
