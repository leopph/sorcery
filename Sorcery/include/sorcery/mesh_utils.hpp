#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
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


struct ComputeMeshletsResult {
  std::vector<MeshletData> meshlets;
  std::vector<std::uint8_t> unique_vertex_indices;
  std::vector<MeshletTriangleData> primitive_indices;
  std::vector<MeshletCullData> cull_data;
};


enum class ComputeMeshletsError : std::uint8_t {
  kInvalidArg,
  kOutOfMemory,
  kUnexpected,
  kUnknown
};


template<typename IdxType, typename PosType>
  requires (std::same_as<IdxType, std::uint16_t> || std::same_as<IdxType, std::uint32_t>)
           && (std::same_as<PosType, Vector3> || std::same_as<PosType, Vector4>)
[[nodiscard]]
auto ComputeMeshlets(
  std::span<IdxType const> indices,
  std::span<PosType const> positions,
  std::uint16_t max_verts_per_meshlet = kMeshletMaxVerts,
  std::uint16_t max_prims_per_meshlet = kMeshletMaxPrims
) -> std::expected<ComputeMeshletsResult, ComputeMeshletsError>;


template<typename IdxType>
  requires std::same_as<IdxType, std::uint16_t> || std::same_as<IdxType, std::uint32_t>
[[nodiscard]]
auto FlattenMeshlets(
  std::span<MeshletData const> meshlets,
  std::span<uint8_t const> unique_vertex_indices,
  std::span<MeshletTriangleData const> primitive_indices
) -> std::vector<IdxType>;
}


#include <sorcery/mesh_utils.inl>
