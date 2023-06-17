#pragma once

#include "Math.hpp"

#include <array>
#include <span>


namespace sorcery {
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


struct Plane {
  float a, b, c, d;

  auto Normalize() noexcept -> void;
  [[nodiscard]] auto Normalized() const noexcept -> Plane;

  [[nodiscard]] auto DistanceToPoint(Vector3 const& p) const noexcept -> float;
};


class Frustum {
  // Their normals point inward
  std::array<Plane, 6> mPlanes{};

public:
  explicit Frustum(Matrix4 const& mtx);

  [[nodiscard]] auto Intersects(BoundingSphere const& boundingSphere) const noexcept -> bool;
  [[nodiscard]] auto Intersects(AABB const& aabb) const noexcept -> bool;
};
}
