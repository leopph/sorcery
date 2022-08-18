#pragma once

#include "Frustum.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"



namespace leopph
{
	struct AABB
	{
		Vector3 min;
		Vector3 max;
	};


	[[nodiscard]] bool is_aabb_in_frustum(AABB const& aabb, Frustum const& frustum, Matrix4 const& modelViewMat);
}
