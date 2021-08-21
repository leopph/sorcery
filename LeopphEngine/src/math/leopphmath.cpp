#include "LeopphMath.hpp"

#include <algorithm>
#include <cmath>


namespace leopph::math
{
	float Pi()
	{
		static float ret{ 2 * std::acosf(0) };
		return ret;
	}


	float ToRadians(const float degrees)
	{
		return degrees * Pi() / 180.0f;
	}


	float ToDegrees(const float radians)
	{
		return radians * 180.0f / Pi();
	}


	float Sin(const float radians)
	{
		return std::sinf(radians);
	}


	float Asin(const float radians)
	{
		return std::asinf(radians);
	}


	float Cos(const float radians)
	{
		return std::cosf(radians);
	}


	float Acos(const float radians)
	{
		return std::acosf(radians);
	}


	float Tan(const float radians)
	{
		return std::tanf(radians);
	}


	float Atan(const float radians)
	{
		return std::atanf(radians);
	}


	float Atan2(const float y, const float x)
	{
		return std::atan2(y, x);
	}


	float Pow(const float base, const float exp)
	{
		return std::powf(base, exp);
	}


	float Sqrt(const float value)
	{
		return std::sqrtf(value);
	}


	float Clamp(const float value, const float min, const float max)
	{
		return std::clamp(value, min, max);
	}


	float Abs(const float value)
	{
		return std::abs(value);
	}
}