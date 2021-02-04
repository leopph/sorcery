#pragma once


#include "leopphapi.h"


namespace leopph
{
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
	};
}