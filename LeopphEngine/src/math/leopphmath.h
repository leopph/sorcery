#pragma once

#include "../api/leopphapi.h"

namespace leopph
{
	/*----------------------------------------------------------------------------------------------------
	The Math class provides functions to help with mathematical computations.
	You can also use other implementations, but prefer these to ensure best performance and compatibility.
	----------------------------------------------------------------------------------------------------*/

	class LEOPPHAPI Math
	{
	public:
		static float Pi();

		static float ToRadians(float degrees);
		static float ToDegrees(float radians);

		static float Sin(float radians);
		static float Asin(float radians);

		static float Cos(float radians);
		static float Acos(float radians);

		static float Tan(float radians);
		static float Atan(float radians);

		static float Pow(float base, float exp);

		static float Sqrt(float value);
	};
}