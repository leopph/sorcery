#include "Time.hpp"

#include <chrono>

namespace leopph {
	namespace {
		std::chrono::steady_clock::time_point gLastFrameTimeMeasurementPoint;
		f32 gLastFrameTimeSeconds;
		f32 gFullTimeSeconds;
	}


	auto init_time() -> void {
		gLastFrameTimeMeasurementPoint = std::chrono::steady_clock::now();
		gLastFrameTimeSeconds = 0;
		gFullTimeSeconds = 0;
	}


	auto measure_time() -> void {
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		std::chrono::nanoseconds delta = now - gLastFrameTimeMeasurementPoint;
		gLastFrameTimeSeconds = std::chrono::duration_cast<std::chrono::duration<f32, std::ratio<1, 1>>>(delta).count();
		gFullTimeSeconds += gLastFrameTimeSeconds;
		gLastFrameTimeMeasurementPoint = now;
	}


	auto get_full_time() noexcept -> f32 {
		return gFullTimeSeconds;
	}


	auto get_frame_time() noexcept -> f32 {
		return gLastFrameTimeSeconds;
	}
}
