#pragma once

#include "Job.hpp"

#include <memory>


namespace leopph::internal
{
	// Returns a job ready to execute the passed function.
	// jobFunc may not be nullptr.
	[[nodiscard]] auto CreateJob(Job::JobFunc jobFunc) -> std::shared_ptr<Job>;

	// Execute the job
	auto Run(std::shared_ptr<Job> job) -> void;
	// Wait for the job to complete.
	// Switches the calling thread to another job if any.
	auto Wait(std::shared_ptr<Job> job) -> void;

	// Close all worker threads.
	auto ShutDownWorkers() -> void;
}
