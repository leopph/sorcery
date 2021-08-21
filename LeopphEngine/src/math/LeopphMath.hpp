#pragma once

#include "../api/leopphapi.h"

namespace leopph
{
	/*----------------------------------------------------------------------------------------------------
	The math namespace provides functions to help with mathematical computations.
	You can also use other implementations, but prefer these to ensure best performance and compatibility.
	----------------------------------------------------------------------------------------------------*/

	namespace math
	{
		[[nodiscard]] LEOPPHAPI float Pi();

		[[nodiscard]] LEOPPHAPI float ToRadians(float degrees);
		[[nodiscard]] LEOPPHAPI float ToDegrees(float radians);

		[[nodiscard]] LEOPPHAPI float Sin(float radians);
		[[nodiscard]] LEOPPHAPI float Asin(float radians);

		[[nodiscard]] LEOPPHAPI float Cos(float radians);
		[[nodiscard]] LEOPPHAPI float Acos(float radians);

		[[nodiscard]] LEOPPHAPI float Tan(float radians);
		[[nodiscard]] LEOPPHAPI float Atan(float radians);

		[[nodiscard]] LEOPPHAPI float Atan2(float y, float x);

		[[nodiscard]] LEOPPHAPI float Pow(float base, float exp);

		[[nodiscard]] LEOPPHAPI float Sqrt(float value);

		[[nodiscard]] LEOPPHAPI float Clamp(float value, float min, float max);

		[[nodiscard]] LEOPPHAPI float Abs(float value);
	};
}