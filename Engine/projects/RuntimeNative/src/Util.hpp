#pragma once

#include "Core.hpp"
#include "Extent2D.hpp"


namespace leopph
{
	template<typename T>
	struct Point2D
	{
		T x;
		T y;
	};

	using Point2DI32 = Point2D<i32>;
	using Point2DI64 = Point2D<i64>;
}