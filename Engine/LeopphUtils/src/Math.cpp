#include "Math.hpp"

#include <algorithm>
#include <cmath>


namespace leopph::math
{
	f32 Pi()
	{
		static auto ret{2 * std::acosf(0)};
		return ret;
	}


	f32 ToRadians(f32 const degrees)
	{
		return degrees * Pi() / 180.0f;
	}


	f32 ToDegrees(f32 const radians)
	{
		return radians * 180.0f / Pi();
	}


	f32 Sin(f32 const radians)
	{
		return std::sinf(radians);
	}


	f32 Asin(f32 const radians)
	{
		return std::asinf(radians);
	}


	f32 Cos(f32 const radians)
	{
		return std::cosf(radians);
	}


	f32 Acos(f32 const radians)
	{
		return std::acosf(radians);
	}


	f32 Tan(f32 const radians)
	{
		return std::tanf(radians);
	}


	f32 Atan(f32 const radians)
	{
		return std::atanf(radians);
	}


	f32 Atan2(f32 const y, f32 const x)
	{
		return std::atan2(y, x);
	}


	f32 Pow(f32 const base, f32 const exp)
	{
		return std::powf(base, exp);
	}


	f32 Sqrt(f32 const value)
	{
		return std::sqrtf(value);
	}


	f32 Log(f32 const value)
	{
		return std::log(value);
	}


	f32 Log2(f32 const value)
	{
		return std::log2(value);
	}


	f32 Clamp(f32 const value, f32 const min, f32 const max)
	{
		return std::clamp(value, min, max);
	}


	f32 Abs(f32 const value)
	{
		return std::abs(value);
	}


	bool IsPowerOfTwo(u32 const value)
	{
		return value != 0 && (value & (value - 1)) == 0;
	}


	u32 NextPowerOfTwo(u32 const value)
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


	f32 Lerp(f32 const from, f32 const to, f32 const t)
	{
		return (1 - t) * from + t * to;
	}


	u8 BinaryDigitCount(u32 const number)
	{
		return static_cast<u8>(Log2(static_cast<f32>(number))) + 1;
	}
}
