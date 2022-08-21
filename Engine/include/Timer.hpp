#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <chrono>


namespace leopph
{
	class Timer
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_last_time() const;
			[[nodiscard]] LEOPPHAPI f32 get_full_time() const;

			LEOPPHAPI void init();
			LEOPPHAPI void tick();

		private:
			std::chrono::time_point<std::chrono::steady_clock> mLastMeasurement{};
			f32 mLastFrameTime{};
			f32 mFullTime{};
	};
}
