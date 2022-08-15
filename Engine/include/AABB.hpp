#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"


namespace leopph
{
	struct AABB
	{
		Vector3 min;
		Vector3 max;

		[[nodiscard]] bool is_visible_in_frustum(Matrix4 const& mvp) const;
	};
}
