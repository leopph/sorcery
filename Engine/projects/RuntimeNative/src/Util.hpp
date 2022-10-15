#pragma once

#include "Core.hpp"


namespace leopph
{
	struct Extent2D
	{
		u32 width;
		u32 height;
	};


	template<typename T>
	struct Point2D
	{
		T x;
		T y;
	};

	using Point2DI32 = Point2D<i32>;
	using Point2DI64 = Point2D<i64>;
}