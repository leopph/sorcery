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


class Frustum {
  // Their normals point inward
  std::array<Vector4, 6> planes_{};

public:
  LEOPPHAPI explicit Frustum(Matrix4 const& mtx);

  [[nodiscard]] auto GetPlanes() const noexcept -> std::span<Vector4 const, 6>;

  [[nodiscard]] LEOPPHAPI auto Intersects(BoundingSphere const& bounding_sphere) const noexcept -> bool;
  [[nodiscard]] LEOPPHAPI auto Intersects(AABB const& aabb) const noexcept -> bool;
};
}
