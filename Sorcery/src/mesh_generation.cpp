#include "mesh_generation.hpp"


namespace sorcery {
auto GenerateSphereMesh(
  float const radius,
  int const latitudes,
  int const longitudes,
  std::vector<Vector3>& vertices,
  std::vector<Vector3>& normals,
  std::vector<Vector2>& uvs,
  std::vector<std::uint32_t>& indices
) -> void {
  // Based on: https://gist.github.com/Pikachuxxxx/5c4c490a7d7679824e0e18af42918efc

  auto const deltaLatitude{PI / static_cast<float>(latitudes)};
  auto const deltaLongitude{2 * PI / static_cast<float>(longitudes)};

  for (auto i = 0; i <= latitudes; i++) {
    auto const latitudeAngle{PI / 2 - static_cast<float>(i) * deltaLatitude};

    float const xz{radius * std::cos(latitudeAngle)};
    float const y{radius * std::sin(latitudeAngle)};

    /* We add (latitudes + 1) vertices per longitude because of equator,
     * the North pole and South pole are not counted here, as they overlap.
     * The first and last vertices have same position and normal, but
     * different tex coords. */
    for (auto j = 0; j <= longitudes; j++) {
      auto const longitudeAngle{static_cast<float>(j) * deltaLongitude};

      auto const x{xz * std::cos(longitudeAngle)};
      auto const z{xz * std::sin(longitudeAngle)};
      auto const u{static_cast<float>(j) / static_cast<float>(longitudes)};
      auto const v{static_cast<float>(i) / static_cast<float>(latitudes)};
      vertices.emplace_back(x, y, z);
      uvs.emplace_back(u, v);
      normals.emplace_back(x / radius, y / radius, z / radius);
    }
  }


  /*  Indices
   *  k1--k1+1
   *  |  / |
   *  | /  |
   *  k2--k2+1 */
  for (auto i = 0; i < latitudes; ++i) {
    unsigned int v1 = i * (longitudes + 1);
    unsigned int v2 = v1 + longitudes + 1;

    // 2 Triangles per latitude block excluding the first and last longitudes blocks
    for (auto j = 0; j < longitudes; j++, v1++, v2++) {
      if (i != 0) {
        indices.push_back(v1);
        indices.push_back(v1 + 1);
        indices.push_back(v2);
      }

      if (i != latitudes - 1) {
        indices.push_back(v1 + 1);
        indices.push_back(v2 + 1);
        indices.push_back(v2);
      }
    }
  }
}
}
