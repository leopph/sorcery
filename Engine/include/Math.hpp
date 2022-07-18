#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"


namespace leopph
{
	// Provides functions to help with mathematical computations.
	// You can also use other implementations, but prefer these to ensure best performance and compatibility.
	namespace math
	{
		[[nodiscard]] auto LEOPPHAPI Pi() -> f32;

		[[nodiscard]] auto LEOPPHAPI ToRadians(f32 degrees) -> f32;
		[[nodiscard]] auto LEOPPHAPI ToDegrees(f32 radians) -> f32;

		[[nodiscard]] auto LEOPPHAPI Sin(f32 radians) -> f32;
		[[nodiscard]] auto LEOPPHAPI Asin(f32 radians) -> f32;

		[[nodiscard]] auto LEOPPHAPI Cos(f32 radians) -> f32;
		[[nodiscard]] auto LEOPPHAPI Acos(f32 radians) -> f32;

		[[nodiscard]] auto LEOPPHAPI Tan(f32 radians) -> f32;
		[[nodiscard]] auto LEOPPHAPI Atan(f32 radians) -> f32;
		[[nodiscard]] auto LEOPPHAPI Atan2(f32 y, f32 x) -> f32;

		[[nodiscard]] auto LEOPPHAPI Pow(f32 base, f32 exp) -> f32;
		[[nodiscard]] auto LEOPPHAPI Sqrt(f32 value) -> f32;

		[[nodiscard]] auto LEOPPHAPI Log(f32 value) -> f32;
		[[nodiscard]] auto LEOPPHAPI Log2(f32 value) -> f32;

		[[nodiscard]] auto LEOPPHAPI Clamp(f32 value, f32 min, f32 max) -> f32;

		[[nodiscard]] auto LEOPPHAPI Abs(f32 value) -> f32;

		[[nodiscard]] auto LEOPPHAPI IsPowerOfTwo(u32 value) -> bool;
		[[nodiscard]] auto LEOPPHAPI NextPowerOfTwo(u32 value) -> u32;

		[[nodiscard]] auto LEOPPHAPI Lerp(f32 from, f32 to, f32 t) -> f32;

		[[nodiscard]] auto LEOPPHAPI BinaryDigitCount(u32 number) -> u8;
	}
}
