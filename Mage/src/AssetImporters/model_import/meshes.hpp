#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <assimp/scene.h>

#include "types.hpp"
#include "sorcery/host/job_system.hpp"
#include "sorcery/host/assets/mesh_import.hpp"


namespace sorcery::mage::model_import {
struct MeshImport {
  std::vector<MeshData> meshes;
  SubassetLabels labels;
  std::optional<HierarchyLayout> hierarchy;
};


[[nodiscard]]
auto ImportMeshes(
  aiScene const& scene,
  std::filesystem::path const& src,
  MeshImportSettings const& settings,
  JobSystem& job_system
) -> std::optional<MeshImport>;

[[nodiscard]]
auto SerializeMesh(
  MeshData const& mesh_data
) -> std::vector<std::byte>;
}
