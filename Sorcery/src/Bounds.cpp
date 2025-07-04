#include "Bounds.hpp"

#include <algorithm>
#include <array>
#include <span>


namespace sorcery {
auto AABB::FromVertices(std::span<Vector3 const> const vertices) noexcept -> AABB {
  AABB bounds{
    .min = Vector3{std::numeric_limits<float>::max()},
    .max = Vector3{std::numeric_limits<float>::lowest()}
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
    Vector3{max[0], min[1], min[2]},
    Vector3{min[0], max[1], min[2]},
    Vector3{max[0], max[1], min[2]},
    Vector3{min[0], min[1], max[2]},
    Vector3{max[0], min[1], max[2]},
    Vector3{min[0], max[1], max[2]},
    max,
  };
}


auto AABB::Transform(Matrix4 const& mtx) const noexcept -> AABB {
  auto vertices{CalculateVertices()};

  for (auto& vertex : vertices) {
    Vector4 transformed{vertex, 1};
    transformed *= mtx;
    transformed /= transformed[3];
    vertex = Vector3{transformed};
  }

  return FromVertices(vertices);
}


Frustum::Frustum(Matrix4 const& mtx) {
  planes_[0][0] = mtx[0][3] + mtx[0][0];
  planes_[0][1] = mtx[1][3] + mtx[1][0];
  planes_[0][2] = mtx[2][3] + mtx[2][0];
  planes_[0][3] = mtx[3][3] + mtx[3][0];

  planes_[1][0] = mtx[0][3] - mtx[0][0];
  planes_[1][1] = mtx[1][3] - mtx[1][0];
  planes_[1][2] = mtx[2][3] - mtx[2][0];
  planes_[1][3] = mtx[3][3] - mtx[3][0];

  planes_[2][0] = mtx[0][3] - mtx[0][1];
  planes_[2][1] = mtx[1][3] - mtx[1][1];
  planes_[2][2] = mtx[2][3] - mtx[2][1];
  planes_[2][3] = mtx[3][3] - mtx[3][1];

  planes_[3][0] = mtx[0][3] + mtx[0][1];
  planes_[3][1] = mtx[1][3] + mtx[1][1];
  planes_[3][2] = mtx[2][3] + mtx[2][1];
  planes_[3][3] = mtx[3][3] + mtx[3][1];

  planes_[4][0] = mtx[0][2];
  planes_[4][1] = mtx[1][2];
  planes_[4][2] = mtx[2][2];
  planes_[4][3] = mtx[3][2];

  planes_[5][0] = mtx[0][3] - mtx[0][2];
  planes_[5][1] = mtx[1][3] - mtx[1][2];
  planes_[5][2] = mtx[2][3] - mtx[2][2];
  planes_[5][3] = mtx[3][3] - mtx[3][2];

  for (auto& plane : planes_) {
    Normalize(plane);
  }
}


auto Frustum::GetPlanes() const noexcept -> std::span<Vector4 const, 6> {
  return planes_;
}


auto Frustum::Intersects(BoundingSphere const& bounding_sphere) const noexcept -> bool {
  auto intersects{true};

  Vector4 const center4{bounding_sphere.center, 1.0F};

  for (auto const& plane : planes_) {
    intersects = intersects && Dot(plane, center4) >= -bounding_sphere.radius;
  }

  return intersects;
}


auto Frustum::Intersects(AABB const& aabb) const noexcept -> bool {
  // This is the proper frustum culling algorithm, the other one only tests if there is a vertex in the positive
  // half space of each plane. This is not correct. The proper solution is added here, but disabled due to producing
  // too many false negatives to be practical to use.
  // TODO improve frustum culling algorithm to eliminate false negatives

  //for (auto const& vertex : aabb.CalculateVertices()) {
  //  auto intersects{true};

  //  for (auto const& plane : planes_) {
  //    intersects = intersects && Dot(plane, Vector4{vertex, 1.0F}) >= 0;
  //  }

  //  if (intersects) {
  //    return true;
  //  }
  //}

  //return false;

  auto intersects{true};

  auto const aabbVertices{aabb.CalculateVertices()};

  for (auto const& plane : planes_) {
    auto distance{std::numeric_limits<float>::lowest()};

    for (auto const& vertex : aabbVertices) {
      distance = std::max(distance, Dot(plane, Vector4{vertex, 1.0F}));
    }

    intersects = intersects && distance >= 0;
  }

  return intersects;
}
}
