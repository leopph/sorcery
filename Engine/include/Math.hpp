#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"


namespace leopph
{
	// Provides functions to help with mathematical computations.
	// You can also use other implementations, but prefer these to ensure best performance and compatibility.
	namespace math
	{
		[[nodiscard]] LEOPPHAPI f32 Pi();

		[[nodiscard]] LEOPPHAPI f32 ToRadians(f32 degrees);
		[[nodiscard]] LEOPPHAPI f32 ToDegrees(f32 radians);

		[[nodiscard]] LEOPPHAPI f32 Sin(f32 radians);
		[[nodiscard]] LEOPPHAPI f32 Asin(f32 radians);

		[[nodiscard]] LEOPPHAPI f32 Cos(f32 radians);
		[[nodiscard]] LEOPPHAPI f32 Acos(f32 radians);

		[[nodiscard]] LEOPPHAPI f32 Tan(f32 radians);
		[[nodiscard]] LEOPPHAPI f32 Atan(f32 radians);
		[[nodiscard]] LEOPPHAPI f32 Atan2(f32 y, f32 x);

		[[nodiscard]] LEOPPHAPI f32 Pow(f32 base, f32 exp);
		[[nodiscard]] LEOPPHAPI f32 Sqrt(f32 value);

		[[nodiscard]] LEOPPHAPI f32 Log(f32 value);
		[[nodiscard]] LEOPPHAPI f32 Log2(f32 value);

		[[nodiscard]] LEOPPHAPI f32 Clamp(f32 value, f32 min, f32 max);

		[[nodiscard]] LEOPPHAPI f32 Abs(f32 value);

		[[nodiscard]] LEOPPHAPI bool IsPowerOfTwo(u32 value);
		[[nodiscard]] LEOPPHAPI u32 NextPowerOfTwo(u32 value);

		[[nodiscard]] LEOPPHAPI f32 Lerp(f32 from, f32 to, f32 t);

		[[nodiscard]] LEOPPHAPI u8 BinaryDigitCount(u32 number);
	}
}
