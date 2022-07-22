#include "threading/JobSystem.hpp"

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
		[[nodiscard]] std::shared_ptr<Job> AllocateJob();

		// Try to get a job from the thread-local queue or steal one
		[[nodiscard]] std::shared_ptr<Job> GetJob();

		// Run the job and mark it completed
		void Execute(std::shared_ptr<Job> job);

		// Windows.h bullshit
		#undef Yield
		// Yield the calling thread's time slice
		void Yield();
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



	std::shared_ptr<Job> CreateJob(Job::JobFunc const jobFunc)
	{
		auto job = AllocateJob();
		job->Func = jobFunc;
		return job;
	}


	void Run(std::shared_ptr<Job> job)
	{
		g_WorkerQueues[THREAD_INDEX].push(std::move(job));
	}


	void Wait(std::shared_ptr<Job> job)
	{
		while (!job->Completed)
		{
			if (auto const nextJob = GetJob())
			{
				Execute(nextJob);
			}
		}
	}


	void InitJobSystem()
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


	void ShutDownJobSystem()
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


	void WorkerFunc()
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
		std::shared_ptr<Job> AllocateJob()
		{
			return std::make_shared_for_overwrite<Job>();
		}


		std::shared_ptr<Job> GetJob()
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


		void Yield()
		{
			_mm_pause();
		}


		void Execute(std::shared_ptr<Job> job)
		{
			job->Func(job->Data);
			job->Completed = true;
		}
	}
}
