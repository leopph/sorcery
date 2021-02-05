#pragma once

#include <chrono>

#include "leopphapi.h"

namespace leopph
{
#pragma warning (push)
#pragma warning (disable: 4251)

	class LEOPPHAPI Time
	{
	public:
		static void OnFrameComplete();
		static float DeltaTime();

	private:
		using Clock = std::chrono::high_resolution_clock;
		using Seconds = std::chrono::duration<float>;
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

		static TimePoint s_LastFrameCompletionTime;
		static Seconds s_LastFrameDeltaTime;
	};

#pragma warning (pop)
}