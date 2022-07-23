#pragma once

#include "Types.hpp"

#include <atomic>
#include <type_traits>


namespace leopph::internal
{
	struct Job
	{
		using JobFunc = void(*)(void const*);

		JobFunc Func;
		std::atomic_bool Completed;
		// Provides padding so that the Job takes up a whole cache line
		// Also acts as storage for Job data
		u8 Data[CACHE_LINE_SIZE - sizeof Func - sizeof Completed];
	};


	static_assert(sizeof(Job) == CACHE_LINE_SIZE);
	static_assert(std::is_standard_layout_v<Job>);
}
