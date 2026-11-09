#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <vector>

#include <assimp/material.h>
#include <assimp/scene.h>

#include "identity.hpp"
#include "types.hpp"
#include "sorcery/host/assets/asset_package.hpp"
#include "sorcery/host/assets/texture_import.hpp"


namespace sorcery::mage::model_import {
[[nodiscard]]
auto IsSrgb(
  aiTextureType tex_type
) -> bool;

// Slot info for each material is always collected. If material import is enabled, asset data is also collected.
// Texture dependencies are also collected if both material and texture import are enabled.
[[nodiscard]]
auto CollectMaterials(
  aiScene const& scene,
  bool import_materials,
  bool import_textures
) -> CollectedMaterials;

// Map texture indices must still be raw tex_infos indices, before the fix-up.
// Returns the alpha threshold each texture should preserve coverage for. 0 means no clip material uses it.
[[nodiscard]]
auto ComputeTextureCoverageThresholds(
  std::span<MaterialAssetData const> materials,
  std::size_t tex_count
) -> std::vector<float>;

// Some textures may not be referenced by any material, but we still want to import them.
// Or we could be skipping materials altogether but still want textures.
auto AppendUnreferencedTextures(
  aiScene const& scene,
  std::vector<SourceTexture>& src_texes
) -> void;

// Converts the raw tex_infos indices of the materials' texture maps into package indices
auto ResolveTextureIds(
  std::span<MaterialAssetData> material_data,
  SubassetIds const& ids
) -> void;

// Fill mesh material slots and remap submesh material slot indices to point into the mesh-local slots.
// A table that describes the remap is returned.
[[nodiscard]]
auto AssignMeshMaterialSlots(
  std::span<MeshData> imported_meshes,
  std::span<MaterialSlotInfo const> mtl_slots
) -> MaterialSlotsByMesh;

[[nodiscard]]
auto BuildTextureResults(
  std::filesystem::path const& src,
  std::span<SourceTexture const> src_texes,
  std::span<float const> tex_coverage_thresholds,
  std::span<TextureImportSettings const> tex_import_settings,
  SubassetIds const& subasset_ids,
  std::vector<AssetImportResult>& out_results
) -> bool;


auto BuildMaterialResults(
  std::span<MaterialAssetData const> material_data,
  std::span<MaterialSlotInfo const> material_slots,
  SubassetIds const& ids, std::vector<AssetImportResult>& out_results
) -> void;
}
