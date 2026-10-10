#pragma once

#include <cstdint>
#include <vector>

#include "Math.hpp"


namespace sorcery {
auto GenerateSphereMesh(
  float radius,
  int latitudes,
  int longitudes,
  std::vector<Vector3>& vertices,
  std::vector<Vector3>& normals,
  std::vector<Vector2>& uvs,
  std::vector<std::uint32_t>& indices
) -> void;
}
