#pragma once

#include <chrono>

namespace leopph::impl
{
	/*----------------------------------------------------------------------
	Internal class that helps measure and produce timer related information.
	----------------------------------------------------------------------*/

	class Timer
	{
	public:
		static void Init();
		static void OnFrameComplete();
		static float DeltaTime();
		static float FullTime();

	private:
		using Clock = std::chrono::high_resolution_clock;
		using Seconds = std::chrono::duration<float>;
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

		static TimePoint s_LastFrameCompletionTime;
		static Seconds s_LastFrameDeltaTime;
		static Seconds s_FullTime;
	};
}