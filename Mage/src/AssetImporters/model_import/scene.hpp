#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>

#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <assimp/vector3.h>

#include "types.hpp"
#include "sorcery/Math.hpp"


namespace sorcery::mage::model_import {
[[nodiscard]]
auto Convert(
  aiVector3D const& ai_vec
) noexcept -> Vector3;

[[nodiscard]]
auto Convert(
  aiQuaternion const& ai_quat
) noexcept -> Quaternion;

[[nodiscard]]
auto Convert(
  aiMatrix4x4 const& ai_mat
) noexcept -> Matrix4;

// Returns nullptr on failure. The importer owns the scene.
[[nodiscard]]
auto LoadScene(
  std::filesystem::path const& src
) -> std::unique_ptr<Assimp::Importer>;

[[nodiscard]]
auto BuildNodeListPreorderDfs(
  aiScene const& scene
) -> NodeListPreorderDfs;

[[nodiscard]]
auto CollectReferringNodesPerMesh(
  std::span<NodeProcessingData const> nodes,
  std::size_t mesh_count
) -> NodesByMesh;

[[nodiscard]]
auto CollectMeshesByGroup(
  NodesByMesh const& nodes_by_mesh,
  std::span<std::optional<MeshProcessingData> const> meshes
) -> MeshesByGroup;

[[nodiscard]]
auto CollectGroupsByNode(
  MeshesByGroup const& meshes_by_group,
  NodesByMesh const& nodes_by_mesh,
  std::size_t node_count
) -> GroupsByNode;
}
