#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include <sorcery/Math.hpp>
#include <sorcery/mesh_data.hpp>


namespace sorcery {
struct SubmeshFaceRange {
  std::size_t first_face;
  std::size_t face_count;
};


struct SubmeshMeshletRange {
  std::size_t first_meshlet;
  std::size_t meshlet_count;
};


template<typename IdxType, typename PosType>
  requires
  (std::same_as<IdxType, std::uint16_t> || std::same_as<IdxType, std::uint32_t>) &&
  (std::same_as<PosType, Vector3> || std::same_as<PosType, Vector4>)
[[nodiscard]]
auto ComputeMeshlets(
  std::span<IdxType const> indices,
  std::span<PosType const> positions,
  std::vector<MeshletData>& out_meshlets,
  std::vector<std::uint8_t>& out_unique_vertex_indices,
  std::vector<MeshletTriangleData>& out_primitive_indices,
  std::vector<MeshletCullData>& out_cull_data,
  std::uint16_t max_verts_per_meshlet = kMeshletMaxVerts,
  std::uint16_t max_prims_per_meshlet = kMeshletMaxPrims
) -> bool;
}


#include <sorcery/mesh_utils.inl>
