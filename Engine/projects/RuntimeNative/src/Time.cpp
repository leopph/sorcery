#include "RuntimeNative.hpp"

#include <chrono>

namespace
{
	std::chrono::steady_clock::time_point gLastFrameTimeMeasurementPoint;
	float gLastFrameTimeSeconds;
	float gFullTimeSeconds;
}


extern "C"
{
	float get_full_time()
	{
		return gFullTimeSeconds;
	}


	float get_frame_time()
	{
		return gLastFrameTimeSeconds;
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
		gLastFrameTimeSeconds = std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1, 1>>>(delta).count();
		gFullTimeSeconds += gLastFrameTimeSeconds;
		gLastFrameTimeMeasurementPoint = now;
	}
}