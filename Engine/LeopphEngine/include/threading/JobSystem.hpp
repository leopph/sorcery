#pragma once

#include "Job.hpp"
#include "Types.hpp"

#include <memory>


namespace leopph::internal::mt
{
	// Returns a job ready to execute the passed function.
	// jobFunc may not be nullptr.
	[[nodiscard]] std::shared_ptr<Job> CreateJob(Job::JobFunc jobFunc);

	// Enqueue the job for execution.
	void Run(std::shared_ptr<Job> job);

	// Wait for the job to complete.
	// Switches the calling thread to another job if any.
	void Wait(std::shared_ptr<Job> job);

	// Initialize the job system.
	void InitJobSystem();

	// Shut down the job system.
	// The job system CANNOT be reinitialized with InitJobSystem after calling this.
	void ShutDownJobSystem();

	// The function run by the workers.
	// This should only be explicitly called by the main thread.
	void WorkerFunc();

	// The number of HW threads.
	extern u8 const NUM_THREADS;
}
