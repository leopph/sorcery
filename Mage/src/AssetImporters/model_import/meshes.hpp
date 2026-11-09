#pragma once

#include <concepts>
#include <filesystem>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

#include <assimp/scene.h>

#include "types.hpp"
#include "sorcery/host/job_system.hpp"


namespace sorcery::mage::model_import {
// Collects every mesh of the scene without any modification
[[nodiscard]]
auto CollectSourceMeshes(
  aiScene const& scene,
  std::filesystem::path const& src
) -> std::vector<std::optional<SourceMesh>>;

// Collects every mesh of the scene with its node's absolute transform baked into its vertices
auto CollectBakedMeshes(
  aiScene const& scene,
  std::filesystem::path const& src,
  std::vector<SourceMesh>& out_meshes,
  std::vector<SourceBone>& out_bone_proc_info
) -> void;

// Fuses meshes that share materials
[[nodiscard]]
auto FuseByMaterial(
  std::span<SourceMesh const> meshes
) -> std::vector<SourceMesh>;


template<class R, class T>
concept SizedRangeOf =
  std::ranges::forward_range<R> &&
  std::ranges::sized_range<R> &&
  std::same_as<std::ranges::range_const_reference_t<R>, T const&>;


// Meshletizes the meshes and appends them to the mesh data as submeshes, one per mesh.
// Fills in the geometry, the submeshes, the bounds and the index format.
[[nodiscard]]
auto BuildMeshGeometry(
  SizedRangeOf<SourceMesh const> auto&& meshes,
  bool force_idx32,
  JobSystem& job_system,
  MeshData& mesh_data
) -> bool;

[[nodiscard]]
auto SerializeMesh(
  MeshData const& mesh_data
) -> std::vector<std::byte>;
}


#include "meshes.inl"
