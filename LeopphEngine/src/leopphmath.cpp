#include <cmath>
#include "leopphmath.h"

namespace leopph
{
	float Math::Pi()
	{
		static float ret{ 2 * std::acosf(0) };
		return ret;
	}


	float Math::ToRadians(float degrees)
	{
		return degrees * Pi() / 180.0f;
	}

	float Math::ToDegrees(float radians)
	{
		return radians * 180.0f / Pi();
	}


	float Math::Sin(float radians)
	{
		return std::sinf(radians);
	}

	float Math::Asin(float radians)
	{
		return std::asinf(radians);
	}


	float Math::Cos(float radians)
	{
		return std::cosf(radians);
	}

	float Math::Acos(float radians)
	{
		return std::acosf(radians);
	}


	float Math::Tan(float radians)
	{
		return std::tanf(radians);
	}

	float Math::Atan(float radians)
	{
		return std::atanf(radians);
	}



	float Math::Pow(float base, float exp)
	{
		return std::powf(base, exp);
	}



	float Math::Sqrt(float value)
	{
		return std::sqrtf(value);
	}
}