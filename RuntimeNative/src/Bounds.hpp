#pragma once

#include "Math.hpp"

#include <array>
#include <span>


namespace leopph {
struct AABB;


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
};


struct BoundingSphere {
	Vector3 center;
	float radius;

	[[nodiscard]] auto IsInFrustum(Frustum const& frustum, Matrix4 const& modelViewMat) const noexcept -> bool;
};


struct AABB {
	Vector3 min;
	Vector3 max;

	[[nodiscard]] static auto FromVertices(std::span<Vector3 const> vertices) noexcept -> AABB;
	[[nodiscard]] auto IsInFrustum(Frustum const& frustum, Matrix4 const& modelViewMat) const noexcept -> bool;
	[[nodiscard]] auto CalculateVertices() const noexcept -> std::array<Vector3, 8>;
};
}
