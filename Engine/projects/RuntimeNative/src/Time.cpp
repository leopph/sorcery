#include "Time.hpp"

#include <chrono>

namespace leopph
{
	namespace
	{
		std::chrono::steady_clock::time_point gLastFrameTimeMeasurementPoint;
		f32 gLastFrameTimeSeconds;
		f32 gFullTimeSeconds;
	}


	void init_time()
	{
		gLastFrameTimeMeasurementPoint = std::chrono::steady_clock::now();
		gLastFrameTimeSeconds = 0;
		gFullTimeSeconds = 0;
	}


	void measure_time()
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		std::chrono::nanoseconds delta = now - gLastFrameTimeMeasurementPoint;
		gLastFrameTimeSeconds = std::chrono::duration_cast<std::chrono::duration<f32, std::ratio<1, 1>>>(delta).count();
		gFullTimeSeconds += gLastFrameTimeSeconds;
		gLastFrameTimeMeasurementPoint = now;
	}


	namespace managedbindings
	{
		f32 get_full_time()
		{
			return gFullTimeSeconds;
		}


		f32 get_frame_time()
		{
			return gLastFrameTimeSeconds;
		}
	}
}