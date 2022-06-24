#include "JobSystem.hpp"

#define WIN32_LEAN_AND_MEAN
#include "Random.hpp"
#include "Types.hpp"
#include "Windows.h"
#include "wsq.hpp"

#include <atomic>
#include <intrin.h>
#include <random>
#include <thread>
#include <utility>
#include <vector>


namespace leopph::internal
{
	/*
	 * Internal functions
	 */

	// Return a new job that is considered empty
	[[nodiscard]] static auto AllocateJob() -> std::shared_ptr<Job>;

	// Global index counter
	[[nodiscard]] static auto GenerateThreadIndex() -> i32;

	// Try to get a job from the thread-local queue or steal one
	[[nodiscard]] static auto GetJob() -> std::shared_ptr<Job>;

	// Return the job queue associated with the calling thread
	[[nodiscard]] auto GetWorkerThreadQueue() -> WorkStealingQueue<std::shared_ptr<Job>>&;

	// Run the job and mark it completed
	static auto Execute(std::shared_ptr<Job> job) -> void;

	// Worker loop
	static auto WorkerFunc() -> void;

	// Windows.h bullshit
	#undef Yield
	// Yield the calling thread's time slice
	static auto Yield() -> void;

	/*
	 * Internal data
	 */

	// The number of HW threads
	static u8 const NUM_THREADS
	{
		[]
		{
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			return static_cast<decltype(NUM_THREADS)>(sysInfo.dwNumberOfProcessors);
		}()
	};

	static u8 const NUM_WORKERS{static_cast<u8>(NUM_THREADS - 1)};

	// NUM_WORKERS number of job queues
	std::vector<WorkStealingQueue<std::shared_ptr<Job>>> g_WorkerQueues{/*NUM_WORKERS*/ NUM_THREADS};

	// Fast global random generator
	XorShift128 g_Random
	{
		[]
		{
			std::random_device rd{};
			return std::array{rd(), rd(), rd(), rd()};
		}()
	};

	// NUM_WORKERS number of threads
	std::vector g_Workers
	{
		[]
		{
			std::vector<std::thread> threads;
			for (u8 i = 0; i < NUM_WORKERS; i++)
			{
				threads.emplace_back(&WorkerFunc);
			}
			return threads;
		}()
	};

	// Per-thread index starting from 0
	thread_local i32 const THREAD_INDEX = GenerateThreadIndex();

	// Sync worker threads to close on exit
	std::atomic_flag g_ShouldClose;

	/*
	 * API implementations
	 */

	auto CreateJob(Job::JobFunc const jobFunc) -> std::shared_ptr<Job>
	{
		auto job = AllocateJob();
		job->Func = jobFunc;
		return job;
	}


	auto Run(std::shared_ptr<Job> job) -> void
	{
		auto& queue = GetWorkerThreadQueue();
		queue.push(std::move(job));
	}


	auto Wait(std::shared_ptr<Job> job) -> void
	{
		while (!job->Completed)
		{
			if (auto const nextJob = GetJob())
			{
				Execute(nextJob);
			}
		}
	}


	auto ShutDownWorkers() -> void
	{
		g_ShouldClose.test_and_set();

		for (auto& worker : g_Workers)
		{
			worker.join();
		}
	}


	/*
	 * Internal implementations
	 */

	auto AllocateJob() -> std::shared_ptr<Job>
	{
		return std::make_shared_for_overwrite<Job>();
	}


	auto GetJob() -> std::shared_ptr<Job>
	{
		auto& queue = GetWorkerThreadQueue();
		auto const job = queue.pop();

		if (!job || !*job)
		{
			auto const index = g_Random() % NUM_WORKERS;
			auto& stealQueue = g_WorkerQueues[index];

			if (&queue == &stealQueue)
			{
				Yield();
				return nullptr;
			}

			auto const stolenJob = stealQueue.steal();

			if (!stolenJob || !*stolenJob)
			{
				Yield();
				return nullptr;
			}

			return *stolenJob;
		}

		return *job;
	}


	auto GetWorkerThreadQueue() -> WorkStealingQueue<std::shared_ptr<Job>>&
	{
		return g_WorkerQueues[THREAD_INDEX];
	}


	auto GenerateThreadIndex() -> i32
	{
		static std::atomic<i32> index = 0;
		return index++;
	}


	auto Yield() -> void
	{
		_mm_pause();
	}


	auto Execute(std::shared_ptr<Job> job) -> void
	{
		job->Func(job->Data);
		job->Completed = true;
	}


	auto WorkerFunc() -> void
	{
		while (!g_ShouldClose.test())
		{
			if (auto const job = GetJob())
			{
				Execute(job);
			}
		}
	}
}
