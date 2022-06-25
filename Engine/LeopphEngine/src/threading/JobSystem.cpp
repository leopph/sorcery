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


namespace leopph::internal::mt
{
	// Internal function declarations ################################################

	namespace
	{
		// Return a new job that is considered empty
		[[nodiscard]] auto AllocateJob() -> std::shared_ptr<Job>;

		// Try to get a job from the thread-local queue or steal one
		[[nodiscard]] auto GetJob() -> std::shared_ptr<Job>;

		// Run the job and mark it completed
		auto Execute(std::shared_ptr<Job> job) -> void;

		// Windows.h bullshit
		#undef Yield
		// Yield the calling thread's time slice
		auto Yield() -> void;
	}



	// Data definitions ##############################################################



	u8 const NUM_THREADS
	{
		[]
		{
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			return static_cast<decltype(NUM_THREADS)>(sysInfo.dwNumberOfProcessors);
		}()
	};


	namespace
	{
		// Fast global random generator
		XorShift128 g_Random
		{
			[]
			{
				std::random_device rd{};
				return std::array{rd(), rd(), rd(), rd()};
			}()
		};

		// Number of additional worker threads along the main thread
		u8 const NUM_WORKERS{static_cast<u8>(NUM_THREADS - 1)};

		// NUM_THREADS number of job queues.
		// WorkStealingQueue cannot be copied or moved so
		// the only way to store them is store keep a constant number of them at all times.
		std::vector<WorkStealingQueue<std::shared_ptr<Job>>> g_WorkerQueues{NUM_THREADS};

		// NUM_WORKERS number of threads.
		// Init fills it and ShutDown clears it.
		std::vector<std::thread> g_Workers;

		// ShutDown must reset this to 1.
		// The main thread always takes 0.
		std::atomic<u8> g_NextWorkerIndex = 0;

		// Per-thread index starting from 0 for the main thread.
		thread_local u8 const THREAD_INDEX = g_NextWorkerIndex++;

		// Syncs worker threads to close on exit.
		std::atomic_flag g_ShouldClose;
	}



	// Public function implementations ##################################################



	auto CreateJob(Job::JobFunc const jobFunc) -> std::shared_ptr<Job>
	{
		auto job = AllocateJob();
		job->Func = jobFunc;
		return job;
	}


	auto Run(std::shared_ptr<Job> job) -> void
	{
		g_WorkerQueues[THREAD_INDEX].push(std::move(job));
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


	auto InitJobSystem() -> void
	{
		g_ShouldClose.clear();

		// We make sure no other threads have swayed the counter.
		g_NextWorkerIndex = 1;

		g_Workers.reserve(NUM_WORKERS);

		for (u8 i = 0; i < NUM_WORKERS; i++)
		{
			g_Workers.emplace_back(&WorkerFunc);
		}
	}


	auto ShutDownJobSystem() -> void
	{
		g_ShouldClose.test_and_set();

		for (auto& worker : g_Workers)
		{
			worker.join();
		}

		g_Workers.clear();

		// Reset index generation to 1.
		// Main thread keeps 0, so we reset to 1.
		g_NextWorkerIndex = 1;

		// Empty all queues.
		for (auto& queue : g_WorkerQueues)
		{
			while (!queue.empty())
			{
				queue.pop();
			}
		}
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



	// Internal function implementations ##########################################



	namespace
	{
		auto AllocateJob() -> std::shared_ptr<Job>
		{
			return std::make_shared_for_overwrite<Job>();
		}


		auto GetJob() -> std::shared_ptr<Job>
		{
			auto& queue = g_WorkerQueues[THREAD_INDEX];
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


		auto Yield() -> void
		{
			_mm_pause();
		}


		auto Execute(std::shared_ptr<Job> job) -> void
		{
			job->Func(job->Data);
			job->Completed = true;
		}
	}
}
