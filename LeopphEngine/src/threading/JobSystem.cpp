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
			m_Threads.emplace_back(ThreadFunc, std::ref(m_Queue), std::ref(m_Lock), std::ref(m_Exit));
		}
	}


	JobSystem::~JobSystem()
	{
		m_Exit = true;

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
		auto ret{job.Future()};
		m_Lock.Lock();
		m_Queue.Push(std::move(job));
		m_Lock.Unlock();
		return ret;
	}


	auto JobSystem::ThreadFunc(std::add_lvalue_reference_t<decltype(m_Queue)> q,
	                           std::add_lvalue_reference_t<decltype(m_Lock)> lock,
	                           const std::atomic_bool& exit) -> void
	{
		while (true)
		{
			lock.Lock();

			if (exit)
			{
				lock.Unlock();
				return;
			}

			if (q.Empty())
			{
				lock.Unlock();
				continue;
			}

			auto job{q.Pop()};
			lock.Unlock();
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
