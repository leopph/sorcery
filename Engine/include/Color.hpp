#pragma once

#include "LeopphApi.hpp"
#include "Math.hpp"
#include "Types.hpp"


namespace leopph
{
	struct Color
	{
		u8 red;
		u8 green;
		u8 blue;
		u8 alpha;


		LEOPPHAPI explicit Color(u8 red = 0, u8 green = 0, u8 blue = 0, u8 alpha = 255);

		LEOPPHAPI explicit Color(Vector4 const& vec);
		LEOPPHAPI explicit operator Vector4() const;
	};
}
