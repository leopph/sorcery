#pragma once
#include "Core.hpp"


namespace leopph::timing {
LEOPPHAPI auto OnApplicationStart() noexcept -> void;
LEOPPHAPI auto OnFrameEnd() noexcept -> void;

// The time in seconds elapsed since starting the application
[[nodiscard]] LEOPPHAPI auto GetFullTime() noexcept -> float;
// The time in seconds it took to complete the previous frame
[[nodiscard]] LEOPPHAPI auto GetFrameTime() noexcept -> float;

// Defaults to -1, which means run as fast as possible
[[nodiscard]] LEOPPHAPI auto GetTargetFrameRate() noexcept -> int;
// Accepts -1 to request running as fast as possible
LEOPPHAPI auto SetTargetFrameRate(int targetFrameRate) noexcept -> void;
}
