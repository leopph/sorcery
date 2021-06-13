#pragma once

#include <chrono>
#include "../api/leopphapi.h"

namespace leopph
{
	// TODO document
	class Time
	{
	public:
		LEOPPHAPI static void Init();
		LEOPPHAPI static void OnFrameComplete();
		LEOPPHAPI static float DeltaTime();
		LEOPPHAPI static float FullTime();

	private:
		using Clock = std::chrono::high_resolution_clock;
		using Seconds = std::chrono::duration<float>;
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

		static TimePoint s_LastFrameCompletionTime;
		static Seconds s_LastFrameDeltaTime;
		static Seconds s_FullTime;
	};
}