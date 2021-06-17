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
		LEOPPHAPI float Pi();

		LEOPPHAPI float ToRadians(float degrees);
		LEOPPHAPI float ToDegrees(float radians);

		LEOPPHAPI float Sin(float radians);
		LEOPPHAPI float Asin(float radians);

		LEOPPHAPI float Cos(float radians);
		LEOPPHAPI float Acos(float radians);

		LEOPPHAPI float Tan(float radians);
		LEOPPHAPI float Atan(float radians);

		LEOPPHAPI float Pow(float base, float exp);

		LEOPPHAPI float Sqrt(float value);
	};
}