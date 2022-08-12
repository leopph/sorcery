#include "Math.hpp"

#include <cmath>


namespace leopph
{
	f32 const PI = 2 * std::acosf(0);



	f32 to_radians(f32 const degrees)
	{
		return degrees * PI / 180.0f;
	}



	f32 to_degrees(f32 const radians)
	{
		return radians * 180.0f / PI;
	}



	bool is_power_of_two(u32 const value)
	{
		return value != 0 && (value & (value - 1)) == 0;
	}



	u32 next_power_of_two(u32 const value)
	{
		if (is_power_of_two(value))
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



	f32 lerp(f32 const from, f32 const to, f32 const t)
	{
		return (1 - t) * from + t * to;
	}



	u8 num_binary_digits(u32 const number)
	{
		return static_cast<u8>(std::log2(static_cast<f32>(number))) + 1;
	}
}
