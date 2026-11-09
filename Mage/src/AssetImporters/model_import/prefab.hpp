#pragma once

#include <span>
#include <string_view>

#include "identity.hpp"
#include "types.hpp"
#include "sorcery/mesh_data.hpp"
#include "sorcery/host/assets/asset_package.hpp"


namespace sorcery::mage::model_import {
[[nodiscard]]
auto CreateHierarchyPrefab(
  std::string_view name,
  std::span<MeshData const> mesh_groups,
  MaterialSlotsByMesh const& mtl_slots_by_mesh,
  HierarchyLayout const& hierarchy,
  SubassetIds const& ids
) -> AssetImportResult;

// Creates a prefab of a single entity that renders the mesh with the materials.
// Placeholder mesh and material objects carry the package indices the prefab's asset references serialize to.
[[nodiscard]]
auto CreateSingleEntityPrefab(
  std::string_view name,
  MeshData const& mesh,
  std::span<SceneMaterialIndex const> mesh_mtl_slot_remap,
  SubassetIds const& ids
) -> AssetImportResult;
}
