#include "LeopphMath.hpp"

#include <algorithm>
#include <cmath>


namespace leopph::math
{
	auto Pi() -> float
	{
		static auto ret{2 * std::acosf(0)};
		return ret;
	}

	auto ToRadians(const float degrees) -> float
	{
		return degrees * Pi() / 180.0f;
	}

	auto ToDegrees(const float radians) -> float
	{
		return radians * 180.0f / Pi();
	}

	auto Sin(const float radians) -> float
	{
		return std::sinf(radians);
	}

	auto Asin(const float radians) -> float
	{
		return std::asinf(radians);
	}

	auto Cos(const float radians) -> float
	{
		return std::cosf(radians);
	}

	auto Acos(const float radians) -> float
	{
		return std::acosf(radians);
	}

	auto Tan(const float radians) -> float
	{
		return std::tanf(radians);
	}

	auto Atan(const float radians) -> float
	{
		return std::atanf(radians);
	}

	auto Atan2(const float y, const float x) -> float
	{
		return std::atan2(y, x);
	}

	auto Pow(const float base, const float exp) -> float
	{
		return std::powf(base, exp);
	}

	auto Sqrt(const float value) -> float
	{
		return std::sqrtf(value);
	}

	auto Clamp(const float value, const float min, const float max) -> float
	{
		return std::clamp(value, min, max);
	}

	auto Abs(const float value) -> float
	{
		return std::abs(value);
	}
}
