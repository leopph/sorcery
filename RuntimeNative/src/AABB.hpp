#pragma once

#include "Util.hpp"
#include "Math.hpp"



namespace leopph {
	struct AABB {
		Vector3 min;
		Vector3 max;
	};


	[[nodiscard]] bool is_aabb_in_frustum(AABB const& aabb, Frustum const& frustum, Matrix4 const& modelViewMat);
}
