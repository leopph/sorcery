#include "JobSystem.hpp"

#include "Windows.h"

#include <algorithm>
#include <utility>


namespace leopph::internal
{
	JobSystem::JobSystem()
	{
		m_Queues[Job::Label::Render] = {};
		m_Queues[Job::Label::Misc] = {};

		m_Workers.reserve(NUM_WORKERS);

		constexpr auto specialWorkerCount{2}; // workers that have manually assigned labels
		static_assert(MIN_WORKER_COUNT >= specialWorkerCount); // make sure we have at least the number of specially assigned workers
		m_Workers.emplace_back(Job::Label::Render, WorkerFunc, std::ref(m_Queues.at(Job::Label::Render)), std::ref(m_Lock), std::ref(m_Exit)); // At least one render worker
		m_Workers.emplace_back(Job::Label::Misc, WorkerFunc, std::ref(m_Queues.at(Job::Label::Misc)), std::ref(m_Lock), std::ref(m_Exit)); // At least one misc worker

		for (auto i = 0ull; i < NUM_WORKERS - specialWorkerCount; ++i)
		{
			m_Workers.emplace_back(Job::Label::Misc, WorkerFunc, std::ref(m_Queues.at(Job::Label::Misc)), std::ref(m_Lock), std::ref(m_Exit)); // Fill with misc worker
		}
	}


	JobSystem::~JobSystem()
	{
		m_Exit = true;

		for (auto& worker : m_Workers)
		{
			worker.join();
		}
	}


	auto JobSystem::Create() -> std::unique_ptr<JobSystem>
	{
		return std::unique_ptr<JobSystem>{new JobSystem{}};
	}


	auto JobSystem::Execute(Job job) -> Job::FutureType
	{
		auto ret{job.Future()};
		const auto label{job.GetLabel()};
		m_Lock.Lock();
		m_Queues.at(label).Push(std::move(job));
		m_Lock.Unlock();
		return ret;
	}


	auto JobSystem::WorkerFunc(std::add_lvalue_reference_t<decltype(m_Queues)::mapped_type> q,
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


	const std::size_t JobSystem::NUM_WORKERS
	{
		[]
		{
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			const std::size_t ret{sysInfo.dwNumberOfProcessors};
			return std::max<std::size_t>(ret, MIN_WORKER_COUNT);
		}()
	};
}
