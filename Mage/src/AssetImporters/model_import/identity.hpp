#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include "types.hpp"


namespace sorcery::mage::model_import {
// Stable identities of the subassets, independent of their order in the package.
// Keys carry a kind prefix ("mesh:", "material:", ...) so they never collide across kinds.
struct SubassetKeys {
  std::string prefab;
  std::vector<std::string> meshes;
  std::vector<std::string> materials;
  std::vector<std::string> textures;
};


class SubassetIds {
public:
  [[nodiscard]] static
  auto FromKeys(SubassetKeys const& keys, std::filesystem::path const& src) -> SubassetIds;

  [[nodiscard]]
  auto GetPrefabId() const -> std::uint64_t;

  [[nodiscard]]
  auto GetMeshId(std::size_t mesh_idx) const -> std::uint64_t;

  [[nodiscard]]
  auto GetMaterialId(std::size_t mtl_idx) const -> std::uint64_t;

  [[nodiscard]]
  auto GetTextureId(std::size_t tex_idx) const -> std::uint64_t;

  [[nodiscard]]
  auto GetMaterialCount() const -> std::size_t;

private:
  std::uint64_t prefab_id_{};
  std::vector<std::uint64_t> mesh_ids_;
  std::vector<std::uint64_t> mtl_ids_;
  std::vector<std::uint64_t> tex_ids_;
};


// Gives every mesh group a key derived from the node hierarchy, so that a group keeps its key
// as long as the node it hangs on keeps its place in the hierarchy.
// It also computes names derived from the same data.
[[nodiscard]]
auto MakeMeshGroupLabels(
  SceneNodeList const& nodes,
  AnchorNodeByGroup const& anchor_node_by_group,
  std::filesystem::path const& src
) -> SubassetLabels;

[[nodiscard]]
auto MakeMaterialLabels(
  std::span<MaterialSlotInfo const> mtl_slots
) -> SubassetLabels;

// Textures are keyed by their file name, or by a hash of their contents if they have none.
// Their names are the file name, or “stem Texture n”.
[[nodiscard]]
auto MakeTextureLabels(
  std::span<SourceTexture const> source_texes,
  std::filesystem::path const& src
) -> SubassetLabels;
}
