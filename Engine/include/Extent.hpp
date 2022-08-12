#pragma once

#include "Types.hpp"


namespace leopph
{
	template<class T = f32>
	struct Extent
	{
		T offsetX;
		T offsetY;
		T width;
		T height;
	};
}
