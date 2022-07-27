#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"
#include "Vector.hpp"


namespace leopph
{
	struct Color
	{
		u8 red;
		u8 green;
		u8 blue;
		u8 alpha;


		explicit Color(u8 red = 255, u8 green = 255, u8 blue = 255, u8 alpha = 255);

		LEOPPHAPI explicit Color(Vector4 const& vec);
		LEOPPHAPI explicit operator Vector4() const;
	};
}
