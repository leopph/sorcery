#include "Bounds.hpp"

#include <algorithm>
#include <array>
#include <span>


namespace leopph {
auto Frustum::Intersects(AABB const& aabb) const noexcept -> bool {
	std::array const frustumNormals{
		Vector3::Forward(),
		Normalize(Cross(Vector3::Up(), rightTopFar - rightTopNear)),
		Normalize(Cross(leftTopFar - leftTopNear, Vector3::Up())),
		Normalize(Cross(Vector3{ 0, rightTopFar[1], rightTopFar[2] } - Vector3{ 0, rightTopNear[1], rightTopNear[2] }, Vector3::Right())),
		Normalize(Cross(Vector3{ 0, rightBottomFar[1], rightBottomFar[2] } - Vector3{ 0, rightBottomNear[1], rightBottomNear[2] }, Vector3::Left()))
	};

	std::array const aabbNormals{
		Vector3::Right(),
		Vector3::Up(),
		Vector3::Forward()
	};

	std::array const frustumEdges{
		Vector3::Right(),
		Vector3::Up(),
		Normalize(rightTopFar - rightTopNear),
		Normalize(leftTopFar - leftTopNear),
		Normalize(rightBottomFar - rightBottomNear),
		Normalize(leftBottomFar - leftBottomNear)
	};

	auto const axesToTest{
		[&frustumNormals, &frustumEdges, &aabbNormals] {
			std::array<Vector3, 26> ret;
			std::ranges::copy(frustumNormals, std::begin(ret));
			std::ranges::copy(aabbNormals, std::begin(ret) + frustumNormals.size());

			for (std::size_t i = 0; i < aabbNormals.size(); i++) {
				for (std::size_t j = 0; j < frustumEdges.size(); j++) {
					ret[frustumNormals.size() + aabbNormals.size() + i * frustumEdges.size() + j] = Cross(aabbNormals[i], frustumEdges[j]);
				}
			}

			return ret;
		}()
	};

	std::array const frustumVertices
	{
		leftBottomFar,
		leftBottomNear,
		leftTopFar,
		leftTopNear,
		rightBottomFar,
		rightBottomNear,
		rightTopFar,
		rightTopNear
	};

	std::array const aabbVertices{ aabb.CalculateVertices() };

	for (auto const& axis : axesToTest) {
		auto frustumProjectionMin{ std::numeric_limits<float>::max() };
		auto frustumProjectionMax{ std::numeric_limits<float>::lowest() };

		for (auto const& frustumVertex : frustumVertices) {
			auto const projection{ ScalarProject(frustumVertex, axis) };
			frustumProjectionMin = std::min(frustumProjectionMin, projection);
			frustumProjectionMax = std::max(frustumProjectionMax, projection);
		}

		auto aabbProjectionMin{ std::numeric_limits<float>::max() };
		auto aabbProjectionMax{ std::numeric_limits<float>::lowest() };

		for (auto const& aabbVertex : aabbVertices) {
			auto const projection{ ScalarProject(aabbVertex, axis) };
			aabbProjectionMin = std::min(aabbProjectionMin, projection);
			aabbProjectionMax = std::max(aabbProjectionMax, projection);
		}

		if (!(frustumProjectionMin <= aabbProjectionMax && aabbProjectionMin <= frustumProjectionMax)) {
			return false;
		}
	}

	return true;
}


auto BoundingSphere::IsInFrustum(Frustum const& frustum, Matrix4 const& modelViewMat) const noexcept -> bool {
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


auto AABB::IsInFrustum(Frustum const& frustum, Matrix4 const& modelViewMat) const noexcept -> bool {
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


auto AABB::CalculateVertices() const noexcept -> std::array<Vector3, 8> {
	return std::array{
		Vector3{ min[0], min[1], min[2] },
		Vector3{ max[0], min[1], min[2] },
		Vector3{ min[0], max[1], min[2] },
		Vector3{ max[0], max[1], min[2] },
		Vector3{ min[0], min[1], max[2] },
		Vector3{ max[0], min[1], max[2] },
		Vector3{ min[0], max[1], max[2] },
		Vector3{ max[0], max[1], max[2] },
	};
}
}
