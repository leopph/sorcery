#include "Bounds.hpp"

#include <algorithm>
#include <array>
#include <span>


namespace leopph {
bool BoundingSphere::IsInFrustum(Frustum const& frustum, Matrix4 const& modelViewMat) const noexcept {
	auto const sphereCenterViewSpace{ Vector3{ Vector4{ center, 1 } * modelViewMat } };
	auto const sphereRadiusViewSpace{ Length(Vector3{ Vector4{ center + Normalize(Vector3{ 1 }) * radius, 1 } * modelViewMat } - sphereCenterViewSpace) };

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

	/*std::array const frustumPlaneNormalsViewSpace{

	};*/

	return true;
}


auto AABB::FromVertices(std::span<Vector3 const> const vertices) noexcept -> AABB {
	AABB bounds{
		.min = Vector3{ std::numeric_limits<float>::max() },
		.max = Vector3{ std::numeric_limits<float>::lowest() }
	};

	for (auto const& vertex : vertices) {
		bounds.min = Min(bounds.min, vertex);
		bounds.max = Max(bounds.max, vertex);
	}

	return bounds;
}


bool AABB::IsInFrustum(Frustum const& frustum, Matrix4 const& modelViewMat) const noexcept {
	auto const boxVerticesViewSpace = [this, &modelViewMat] {
		std::array boxVertices
		{
			Vector3{ this->min[0], this->min[1], this->min[2] },
			Vector3{ this->max[0], this->min[1], this->min[2] },
			Vector3{ this->min[0], this->max[1], this->min[2] },
			Vector3{ this->max[0], this->max[1], this->min[2] },
			Vector3{ this->min[0], this->min[1], this->max[2] },
			Vector3{ this->max[0], this->min[1], this->max[2] },
			Vector3{ this->min[0], this->max[1], this->max[2] },
			Vector3{ this->max[0], this->max[1], this->max[2] },
		};

		for (auto& vertex : boxVertices) {
			vertex = Vector3{ Vector4{ vertex, 1 } * modelViewMat };
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

	auto const boxAxesViewSpace = [&modelViewMat] {
		std::array boxAxes
		{
			Vector3::Right(),
			Vector3::Up(),
			Vector3::Forward()
		};

		for (auto& axis : boxAxes) {
			axis = Normalized(Vector3{ Vector4{ axis, 0 } * modelViewMat });
		}

		return boxAxes;
	}();


	// Tests whether the bounding box and the frustum overlap when projected onto the passed axes
	auto const overlapOnAxes = [&boxVerticesViewSpace, &frustumVerticesViewSpace](std::span<Vector3 const> const axes) {
		for (auto const& normal : axes) {
			auto boxMin = std::numeric_limits<float>::max();
			auto boxMax = std::numeric_limits<float>::lowest();

			for (auto const& vertex : boxVerticesViewSpace) {
				auto const proj = Dot(vertex, normal);
				boxMin = std::min(proj, boxMin);
				boxMax = std::max(proj, boxMax);
			}

			auto frustMin = std::numeric_limits<float>::max();
			auto frustMax = std::numeric_limits<float>::lowest();

			for (auto const& vertex : frustumVerticesViewSpace) {
				auto const proj = Dot(vertex, normal);
				frustMin = std::min(proj, frustMin);
				frustMax = std::max(proj, frustMax);
			}

			if (frustMax < boxMin || boxMax < frustMin) {
				return false;
			}
		}

		return true;
	};


	// Project onto box normals
	// Box normals are equal to its axes
	if (!overlapOnAxes(boxAxesViewSpace)) {
		return false;
	}

	// Project onto frustum normals
	if (std::array const frustumNormalsViewSpace
	{
		Vector3::Forward(),
		Normalized(Vector3{ frustum.rightTopNear[2], 0, -frustum.rightTopNear[0] }),
		Normalized(Vector3{ frustum.leftTopNear[2], 0, frustum.leftTopNear[0] }),
		Normalized(Vector3{ 0, frustum.rightTopNear[2], -frustum.rightTopNear[1] }),
		Normalized(Vector3{ 0, -frustum.rightBottomNear[2], frustum.rightBottomNear[1] })
	}; !overlapOnAxes(frustumNormalsViewSpace)) {
		return false;
	}

	// Project onto cross products between box edges and frustum edges
	// Box edges are equal to its axes
	{
		std::array const frustumEdgesViewSpace
		{
			Vector3::Right(),
			Vector3::Up(),
			Normalized(frustum.rightBottomNear),
			Normalized(frustum.leftBottomNear),
			Normalized(frustum.leftTopNear),
			Normalized(frustum.rightTopNear)
		};

		std::array<Vector3, boxAxesViewSpace.size() * frustumEdgesViewSpace.size()> crossProducts;

		for (std::size_t i = 0; i < boxAxesViewSpace.size(); i++) {
			for (std::size_t j = 0; j < frustumEdgesViewSpace.size(); j++) {
				crossProducts[i * frustumEdgesViewSpace.size() + j] = Cross(boxAxesViewSpace[i], frustumEdgesViewSpace[i]);
			}
		}

		if (!overlapOnAxes(crossProducts)) {
			return false;
		}
	}

	// Haven't found a separating axis, box and frustum must overlap
	return true;
}
}
