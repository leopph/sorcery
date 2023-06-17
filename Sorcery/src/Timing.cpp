#include "Timing.hpp"

#include <chrono>


namespace sorcery::timing {
namespace {
std::chrono::steady_clock::time_point gLastMeasurePoint;
std::chrono::nanoseconds gLastFrameTime;
std::chrono::nanoseconds gFullTime;

float gLastFrameTimeSeconds;
float gFullTimeSeconds;

int gTargetFrameRate{ -1 };
std::chrono::nanoseconds gMinFrameTime{ 0 };
}


auto OnApplicationStart() noexcept -> void {
  gLastMeasurePoint = std::chrono::steady_clock::now();
  gLastFrameTimeSeconds = 0;
  gFullTimeSeconds = 0;
}


auto OnFrameEnd() noexcept -> void {
  do {
    gLastFrameTime = std::chrono::steady_clock::now() - gLastMeasurePoint;
  } while (gLastFrameTime < gMinFrameTime);

  gLastMeasurePoint += gLastFrameTime;
  gFullTime += gLastFrameTime;

  using FloatSeconds = std::chrono::duration<f32, std::ratio<1, 1>>;
  gLastFrameTimeSeconds = std::chrono::duration_cast<FloatSeconds>(gLastFrameTime).count();
  gFullTimeSeconds = std::chrono::duration_cast<FloatSeconds>(gFullTime).count();
}


auto GetFullTime() noexcept -> float {
  return gFullTimeSeconds;
}


auto GetFrameTime() noexcept -> float {
  return gLastFrameTimeSeconds;
}


auto GetTargetFrameRate() noexcept -> int {
  return gTargetFrameRate;
}


auto SetTargetFrameRate(int const targetFrameRate) noexcept -> void {
  gTargetFrameRate = targetFrameRate;
  gMinFrameTime = std::chrono::nanoseconds{ static_cast<int>(1e9f / static_cast<float>(targetFrameRate)) };
}
}
