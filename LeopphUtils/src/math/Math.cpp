#include "Math.hpp"

#include <algorithm>
#include <cmath>


namespace leopph::math
{
	auto Pi() -> float
	{
		static auto ret{2 * std::acosf(0)};
		return ret;
	}


	auto ToRadians(float const degrees) -> float
	{
		return degrees * Pi() / 180.0f;
	}


	auto ToDegrees(float const radians) -> float
	{
		return radians * 180.0f / Pi();
	}


	auto Sin(float const radians) -> float
	{
		return std::sinf(radians);
	}


	auto Asin(float const radians) -> float
	{
		return std::asinf(radians);
	}


	auto Cos(float const radians) -> float
	{
		return std::cosf(radians);
	}


	auto Acos(float const radians) -> float
	{
		return std::acosf(radians);
	}


	auto Tan(float const radians) -> float
	{
		return std::tanf(radians);
	}


	auto Atan(float const radians) -> float
	{
		return std::atanf(radians);
	}


	auto Atan2(float const y, float const x) -> float
	{
		return std::atan2(y, x);
	}


	auto Pow(float const base, float const exp) -> float
	{
		return std::powf(base, exp);
	}


	auto Sqrt(float const value) -> float
	{
		return std::sqrtf(value);
	}


	auto Clamp(float const value, float const min, float const max) -> float
	{
		return std::clamp(value, min, max);
	}


	auto Abs(float const value) -> float
	{
		return std::abs(value);
	}


	auto IsPowerOfTwo(unsigned const value) -> bool
	{
		return value != 0 && (value & (value - 1)) == 0;
	}


	auto NextPowerOfTwo(unsigned const value) -> unsigned
	{
		if (IsPowerOfTwo(value))
		{
			return value;
		}

		unsigned ret{1};

		while (ret < value)
		{
			ret <<= 1;
		}

		return ret;
	}


	auto Lerp(float const from, float const to, float const t) -> float
	{
		return (1 - t) * from + t * to;
	}
}
