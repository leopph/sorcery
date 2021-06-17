#include <cmath>
#include "leopphmath.h"

namespace leopph::math
{
	float Pi()
	{
		static float ret{ 2 * std::acosf(0) };
		return ret;
	}


	float ToRadians(float degrees)
	{
		return degrees * Pi() / 180.0f;
	}

	float ToDegrees(float radians)
	{
		return radians * 180.0f / Pi();
	}


	float Sin(float radians)
	{
		return std::sinf(radians);
	}

	float Asin(float radians)
	{
		return std::asinf(radians);
	}


	float Cos(float radians)
	{
		return std::cosf(radians);
	}

	float Acos(float radians)
	{
		return std::acosf(radians);
	}


	float Tan(float radians)
	{
		return std::tanf(radians);
	}

	float Atan(float radians)
	{
		return std::atanf(radians);
	}



	float Pow(float base, float exp)
	{
		return std::powf(base, exp);
	}



	float Sqrt(float value)
	{
		return std::sqrtf(value);
	}
}