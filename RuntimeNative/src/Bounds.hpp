#pragma once

#include "Math.hpp"

#include <array>
#include <span>


namespace leopph {
struct BoundingSphere {
	Vector3 center;
	float radius;
};


struct AABB {
	Vector3 min;
	Vector3 max;

	[[nodiscard]] static auto FromVertices(std::span<Vector3 const> vertices) noexcept -> AABB;
	[[nodiscard]] auto CalculateVertices() const noexcept -> std::array<Vector3, 8>;
};


struct Frustum {
	Vector3 rightTopNear;
	Vector3 leftTopNear;
	Vector3 leftBottomNear;
	Vector3 rightBottomNear;
	Vector3 rightTopFar;
	Vector3 leftTopFar;
	Vector3 leftBottomFar;
	Vector3 rightBottomFar;

	[[nodiscard]] auto Intersects(AABB const& aabb) const noexcept -> bool;
	[[nodiscard]] auto Intersects(BoundingSphere const& boundingSphere) const noexcept -> bool;
};
}
