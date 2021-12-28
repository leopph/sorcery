#pragma once

#include "../api/LeopphApi.hpp"


namespace leopph
{
	// Provides functions to help with mathematical computations.
	// You can also use other implementations, but prefer these to ensure best performance and compatibility.
	namespace math
	{
		[[nodiscard]] LEOPPHAPI auto Pi() -> float;

		[[nodiscard]] LEOPPHAPI auto ToRadians(float degrees) -> float;
		[[nodiscard]] LEOPPHAPI auto ToDegrees(float radians) -> float;

		[[nodiscard]] LEOPPHAPI auto Sin(float radians) -> float;
		[[nodiscard]] LEOPPHAPI auto Asin(float radians) -> float;

		[[nodiscard]] LEOPPHAPI auto Cos(float radians) -> float;
		[[nodiscard]] LEOPPHAPI auto Acos(float radians) -> float;

		[[nodiscard]] LEOPPHAPI auto Tan(float radians) -> float;
		[[nodiscard]] LEOPPHAPI auto Atan(float radians) -> float;

		[[nodiscard]] LEOPPHAPI auto Atan2(float y, float x) -> float;

		[[nodiscard]] LEOPPHAPI auto Pow(float base, float exp) -> float;

		[[nodiscard]] LEOPPHAPI auto Sqrt(float value) -> float;

		[[nodiscard]] LEOPPHAPI auto Clamp(float value, float min, float max) -> float;

		[[nodiscard]] LEOPPHAPI auto Abs(float value) -> float;
	};
}
