#pragma once

#include "Vector.hpp"


namespace leopph
{
	struct Frustum
	{
		Vector3 rightTopNear;
		Vector3 leftTopNear;
		Vector3 leftBottomNear;
		Vector3 rightBottomNear;
		Vector3 rightTopFar;
		Vector3 leftTopFar;
		Vector3 leftBottomFar;
		Vector3 rightBottomFar;
	};
}
