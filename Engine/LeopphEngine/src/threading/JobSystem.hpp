#pragma once

#include "Job.hpp"
#include "Types.hpp"

#include <memory>


namespace leopph::internal::mt
{
	// Returns a job ready to execute the passed function.
	// jobFunc may not be nullptr.
	[[nodiscard]] auto CreateJob(Job::JobFunc jobFunc) -> std::shared_ptr<Job>;

	// Enqueue the job for execution.
	auto Run(std::shared_ptr<Job> job) -> void;

	// Wait for the job to complete.
	// Switches the calling thread to another job if any.
	auto Wait(std::shared_ptr<Job> job) -> void;

	// Initialize the job system.
	auto InitJobSystem() -> void;

	// Shut down the job system.
	// The job system CANNOT be reinitialized with InitJobSystem after calling this.
	auto ShutDownJobSystem() -> void;

	// The function run by the workers.
	// This should only be explicitly called by the main thread.
	auto WorkerFunc() -> void;

	// The number of HW threads.
	extern u8 const NUM_THREADS;
}
