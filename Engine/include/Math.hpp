#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"


namespace leopph
{
		LEOPPHAPI extern f32 const PI;

		[[nodiscard]] LEOPPHAPI f32 to_radians(f32 degrees);
		[[nodiscard]] LEOPPHAPI f32 to_degrees(f32 radians);

		[[nodiscard]] LEOPPHAPI bool is_power_of_two(u32 value);
		[[nodiscard]] LEOPPHAPI u32 next_power_of_two(u32 value);

		[[nodiscard]] LEOPPHAPI f32 lerp(f32 from, f32 to, f32 t);

		[[nodiscard]] LEOPPHAPI u8 num_binary_digits(u32 number);
}
