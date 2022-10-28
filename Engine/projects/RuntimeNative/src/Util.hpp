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


	struct Viewport
	{
		Point2D<u32> position;
		Extent2D extent;
	};


	struct NormalizedViewport
	{
		f32 posX;
		f32 posY;
		f32 extentX;
		f32 extentY;
	};
}