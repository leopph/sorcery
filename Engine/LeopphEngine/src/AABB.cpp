#include "AABB.hpp"


namespace leopph
{
	bool AABB::is_visible_in_frustum(Matrix4 const& mvp) const
	{
		for (std::array const boxVertices
		     {
			     Vector4{min[0], min[1], min[2], 1.0},
			     Vector4{max[0], min[1], min[2], 1.0},
			     Vector4{min[0], max[1], min[2], 1.0},
			     Vector4{max[0], max[1], min[2], 1.0},
			     Vector4{min[0], min[1], max[2], 1.0},
			     Vector4{max[0], min[1], max[2], 1.0},
			     Vector4{min[0], max[1], max[2], 1.0},
			     Vector4{max[0], max[1], max[2], 1.0},
		     };
		     auto const& boxVertex : boxVertices)
		{
			if (auto const boxVertexWorld = boxVertex * mvp;
				-boxVertexWorld[3] <= boxVertexWorld[0] && boxVertexWorld[0] <= boxVertexWorld[3] &&
				-boxVertexWorld[3] <= boxVertexWorld[1] && boxVertexWorld[1] <= boxVertexWorld[3] &&
				-boxVertexWorld[3] <= boxVertexWorld[2] && boxVertexWorld[2] <= boxVertexWorld[3])
			{
				return true;
			}
		}

		return false;
	}
}
