#include "Bounds.hpp"

#include <algorithm>
#include <array>
#include <span>


namespace sorcery {
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
    min,
    Vector3{ max[0], min[1], min[2] },
    Vector3{ min[0], max[1], min[2] },
    Vector3{ max[0], max[1], min[2] },
    Vector3{ min[0], min[1], max[2] },
    Vector3{ max[0], min[1], max[2] },
    Vector3{ min[0], max[1], max[2] },
    max,
  };
}


auto Plane::Normalize() noexcept -> void {
  auto const normalLength{ std::sqrt(std::pow(a, 2.0f) + std::pow(b, 2.0f) + std::pow(c, 2.0f)) };
  a /= normalLength;
  b /= normalLength;
  c /= normalLength;
  d /= normalLength;
}


auto Plane::Normalized() const noexcept -> Plane {
  auto ret{ *this };
  ret.Normalize();
  return ret;
}


auto Plane::DistanceToPoint(Vector3 const& p) const noexcept -> float {
  return a * p[0] + b * p[1] + c * p[2] + d;
}


Frustum::Frustum(Matrix4 const& mtx) {
  mPlanes[0].a = mtx[0][3] + mtx[0][0];
  mPlanes[0].b = mtx[1][3] + mtx[1][0];
  mPlanes[0].c = mtx[2][3] + mtx[2][0];
  mPlanes[0].d = mtx[3][3] + mtx[3][0];

  mPlanes[1].a = mtx[0][3] - mtx[0][0];
  mPlanes[1].b = mtx[1][3] - mtx[1][0];
  mPlanes[1].c = mtx[2][3] - mtx[2][0];
  mPlanes[1].d = mtx[3][3] - mtx[3][0];

  mPlanes[2].a = mtx[0][3] - mtx[0][1];
  mPlanes[2].b = mtx[1][3] - mtx[1][1];
  mPlanes[2].c = mtx[2][3] - mtx[2][1];
  mPlanes[2].d = mtx[3][3] - mtx[3][1];

  mPlanes[3].a = mtx[0][3] + mtx[0][1];
  mPlanes[3].b = mtx[1][3] + mtx[1][1];
  mPlanes[3].c = mtx[2][3] + mtx[2][1];
  mPlanes[3].d = mtx[3][3] + mtx[3][1];

  mPlanes[4].a = mtx[0][2];
  mPlanes[4].b = mtx[1][2];
  mPlanes[4].c = mtx[2][2];
  mPlanes[4].d = mtx[3][2];

  mPlanes[5].a = mtx[0][3] - mtx[0][2];
  mPlanes[5].b = mtx[1][3] - mtx[1][2];
  mPlanes[5].c = mtx[2][3] - mtx[2][2];
  mPlanes[5].d = mtx[3][3] - mtx[3][2];

  for (auto& plane : mPlanes) {
    plane.Normalize();
  }
}


auto Frustum::Intersects(BoundingSphere const& boundingSphere) const noexcept -> bool {
  auto intersects{ true };

  for (auto const& plane : mPlanes) {
    intersects = intersects && plane.DistanceToPoint(boundingSphere.center) >= -boundingSphere.radius;
  }

  return intersects;
}


auto Frustum::Intersects(AABB const& aabb) const noexcept -> bool {
  auto intersects{ true };

  auto const aabbVertices{ aabb.CalculateVertices() };

  for (auto const& plane : mPlanes) {
    auto distance{ std::numeric_limits<float>::lowest() };

    for (auto const& vertex : aabbVertices) {
      distance = std::max(distance, plane.DistanceToPoint(vertex));
    }

    intersects = intersects && distance >= 0;
  }

  return intersects;
}
}
