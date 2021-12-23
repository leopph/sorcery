#pragma once

#include <chrono>


namespace leopph::internal
{
	/*----------------------------------------------------------------------
	Internal class that helps measure and produce timer related information.
	----------------------------------------------------------------------*/

	class Timer
	{
		public:
			static auto Init() -> void;
			static auto OnFrameComplete() -> void;
			static auto DeltaTime() -> float;
			static auto FullTime() -> float;

		private:
			using Clock = std::chrono::high_resolution_clock;
			using Seconds = std::chrono::duration<float>;
			using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

			static TimePoint s_LastFrameCompletionTime;
			static Seconds s_LastFrameDeltaTime;
			static Seconds s_FullTime;
	};
}
