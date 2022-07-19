#include "Math.hpp"

#include <algorithm>
#include <cmath>


namespace leopph::math
{
	auto Pi() -> f32
	{
		static auto ret{2 * std::acosf(0)};
		return ret;
	}


	auto ToRadians(f32 const degrees) -> f32
	{
		return degrees * Pi() / 180.0f;
	}


	auto ToDegrees(f32 const radians) -> f32
	{
		return radians * 180.0f / Pi();
	}


	auto Sin(f32 const radians) -> f32
	{
		return std::sinf(radians);
	}


	auto Asin(f32 const radians) -> f32
	{
		return std::asinf(radians);
	}


	auto Cos(f32 const radians) -> f32
	{
		return std::cosf(radians);
	}


	auto Acos(f32 const radians) -> f32
	{
		return std::acosf(radians);
	}


	auto Tan(f32 const radians) -> f32
	{
		return std::tanf(radians);
	}


	auto Atan(f32 const radians) -> f32
	{
		return std::atanf(radians);
	}


	auto Atan2(f32 const y, f32 const x) -> f32
	{
		return std::atan2(y, x);
	}


	auto Pow(f32 const base, f32 const exp) -> f32
	{
		return std::powf(base, exp);
	}


	auto Sqrt(f32 const value) -> f32
	{
		return std::sqrtf(value);
	}

	auto Log(f32 const value) -> f32
	{
		return std::log(value);
	}

	auto Log2(f32 const value) -> f32
	{
		return std::log2(value);
	}


	auto Clamp(f32 const value, f32 const min, f32 const max) -> f32
	{
		return std::clamp(value, min, max);
	}


	auto Abs(f32 const value) -> f32
	{
		return std::abs(value);
	}


	auto IsPowerOfTwo(u32 const value) -> bool
	{
		return value != 0 && (value & (value - 1)) == 0;
	}


	auto NextPowerOfTwo(u32 const value) -> u32
	{
		if (IsPowerOfTwo(value))
		{
			return value;
		}

		u32 ret{1};

		while (ret < value)
		{
			ret <<= 1;
		}

		return ret;
	}


	auto Lerp(f32 const from, f32 const to, f32 const t) -> f32
	{
		return (1 - t) * from + t * to;
	}

	auto BinaryDigitCount(u32 const number) -> u8
	{
		return static_cast<u8>(Log2(static_cast<f32>(number))) + 1;
	}

}
