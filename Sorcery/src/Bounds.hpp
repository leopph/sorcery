#pragma once

#include <array>
#include <span>

#include "Core.hpp"
#include "Math.hpp"


namespace sorcery {
struct BoundingSphere {
  Vector3 center;
  float radius;
};


struct AABB {
  Vector3 min;
  Vector3 max;

  [[nodiscard]] LEOPPHAPI static auto FromVertices(std::span<Vector3 const> vertices) noexcept -> AABB;
  [[nodiscard]] LEOPPHAPI auto CalculateVertices() const noexcept -> std::array<Vector3, 8>;
  [[nodiscard]] LEOPPHAPI auto Transform(Matrix4 const& mtx) const noexcept -> AABB;
};


struct Plane {
  float a, b, c, d;

  LEOPPHAPI auto Normalize() noexcept -> void;
  [[nodiscard]] LEOPPHAPI auto Normalized() const noexcept -> Plane;

  [[nodiscard]] LEOPPHAPI auto DistanceToPoint(Vector3 const& p) const noexcept -> float;
};


class Frustum {
  // Their normals point inward
  std::array<Plane, 6> mPlanes{};

public:
  LEOPPHAPI explicit Frustum(Matrix4 const& mtx);

  [[nodiscard]] LEOPPHAPI auto Intersects(BoundingSphere const& boundingSphere) const noexcept -> bool;
  [[nodiscard]] LEOPPHAPI auto Intersects(AABB const& aabb) const noexcept -> bool;
};
}
