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


auto Frustum::Intersects(BoundingSphere const& boundingSphere) const noexcept -> bool {
	return true; // TODO
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
