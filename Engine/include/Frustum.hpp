#pragma once

#include "Vector.hpp"


namespace leopph
{
	struct Frustum
	{
		Vector3 NearTopLeft;
		Vector3 NearBottomLeft;
		Vector3 NearBottomRight;
		Vector3 NearTopRight;
		Vector3 FarTopLeft;
		Vector3 FarBottomLeft;
		Vector3 FarBottomRight;
		Vector3 FarTopRight;
	};
}
