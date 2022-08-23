#include "AABB.hpp"

#include "Types.hpp"

#include <algorithm>
#include <array>
#include <span>


namespace leopph
{
	bool is_aabb_in_frustum(AABB const& aabb, Frustum const& frustum, Matrix4 const& modelViewMat)
	{
		auto const boxVerticesViewSpace = [&aabb, &modelViewMat]
		{
			std::array boxVertices
			{
				Vector3{aabb.min[0], aabb.min[1], aabb.min[2]},
				Vector3{aabb.max[0], aabb.min[1], aabb.min[2]},
				Vector3{aabb.min[0], aabb.max[1], aabb.min[2]},
				Vector3{aabb.max[0], aabb.max[1], aabb.min[2]},
				Vector3{aabb.min[0], aabb.min[1], aabb.max[2]},
				Vector3{aabb.max[0], aabb.min[1], aabb.max[2]},
				Vector3{aabb.min[0], aabb.max[1], aabb.max[2]},
				Vector3{aabb.max[0], aabb.max[1], aabb.max[2]},
			};

			for (auto& vertex : boxVertices)
			{
				vertex = Vector3{Vector4{vertex, 1} * modelViewMat};
			}

			return boxVertices;
		}();

		std::array const frustumVerticesViewSpace
		{
			frustum.leftBottomFar,
			frustum.leftBottomNear,
			frustum.leftTopFar,
			frustum.leftTopNear,
			frustum.rightBottomFar,
			frustum.rightBottomNear,
			frustum.rightTopFar,
			frustum.rightTopNear
		};

		auto const boxAxesViewSpace = [&modelViewMat]
		{
			std::array boxAxes
			{
				Vector3::right(),
				Vector3::up(),
				Vector3::forward()
			};

			for (auto& axis : boxAxes)
			{
				axis = Vector3{Vector4{axis, 0} * modelViewMat}.normalize();
			}

			return boxAxes;
		}();


		// Tests whether the bounding box and the frustum overlap when projected onto the passed axes
		auto const overlapOnAxes = [&boxVerticesViewSpace, &frustumVerticesViewSpace](std::span<Vector3 const> const axes)
		{
			for (auto const& normal : axes)
			{
				auto boxMin = std::numeric_limits<f32>::max();
				auto boxMax = std::numeric_limits<f32>::lowest();

				for (auto const& vertex : boxVerticesViewSpace)
				{
					auto const proj = dot(vertex, normal);
					boxMin = std::min(proj, boxMin);
					boxMax = std::max(proj, boxMax);
				}

				auto frustMin = std::numeric_limits<f32>::max();
				auto frustMax = std::numeric_limits<f32>::lowest();

				for (auto const& vertex : frustumVerticesViewSpace)
				{
					auto const proj = dot(vertex, normal);
					frustMin = std::min(proj, frustMin);
					frustMax = std::max(proj, frustMax);
				}

				if (frustMax < boxMin || boxMax < frustMin)
				{
					return false;
				}
			}

			return true;
		};


		// Project onto box normals
		// Box normals are equal to its axes
		if (!overlapOnAxes(boxAxesViewSpace))
		{
			return false;
		}

		// Project onto frustum normals
		if (std::array const frustumNormalsViewSpace
		{
			Vector3::forward(),
			Vector3{frustum.rightTopNear[2], 0, -frustum.rightTopNear[0]}.normalize(),
			Vector3{frustum.leftTopNear[2], 0, frustum.leftTopNear[0]}.normalize(),
			Vector3{0, frustum.rightTopNear[2], -frustum.rightTopNear[1]}.normalize(),
			Vector3{0, -frustum.rightBottomNear[2], frustum.rightBottomNear[1]}.normalize()
		}; !overlapOnAxes(frustumNormalsViewSpace))
		{
			return false;
		}

		// Project onto cross products between box edges and frustum edges
		// Box edges are equal to its axes
		{
			std::array const frustumEdgesViewSpace
			{
				Vector3::right(),
				Vector3::up(),
				frustum.rightBottomNear.normalized(),
				frustum.leftBottomNear.normalized(),
				frustum.leftTopNear.normalized(),
				frustum.rightTopNear.normalized()
			};

			std::array<Vector3, boxAxesViewSpace.size() * frustumEdgesViewSpace.size()> crossProducts;

			for (std::size_t i = 0; i < boxAxesViewSpace.size(); i++)
			{
				for (std::size_t j = 0; j < frustumEdgesViewSpace.size(); j++)
				{
					crossProducts[i * frustumEdgesViewSpace.size() + j] = cross(boxAxesViewSpace[i], frustumEdgesViewSpace[i]);
				}
			}

			if (!overlapOnAxes(crossProducts))
			{
				return false;
			}
		}

		// Haven't found a separating axis, box and frustum must overlap
		return true;
	}
}
