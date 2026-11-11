#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/texture.h>

#include "sorcery/Math.hpp"
#include "sorcery/mesh_data.hpp"
#include "sorcery/not_null.hpp"
#include "sorcery/observer_ptr.hpp"
#include "sorcery/host/assets/material_asset.hpp"


namespace sorcery::mage::model_import {
struct SourceTexture {
  aiTexture const* tex;
  aiTextureType type;

  [[nodiscard]]
  auto operator==(SourceTexture const& other) const -> bool = default;
};


struct CollectedMaterials {
  std::vector<MaterialSlotInfo> slots;
  std::vector<MaterialAssetData> assets;
  std::vector<SourceTexture> textures;
};


struct SourceMesh {
  std::vector<Vector3> positions;
  std::vector<Vector3> normals;
  std::vector<Vector2> uvs;
  std::vector<Vector3> tangents;
  std::vector<unsigned> indices;
  std::vector<Vector4> bone_weights;
  std::vector<Vector<std::uint32_t, 4>> bone_indices;
  unsigned mtl_idx{};
};


struct SourceBone {
  Matrix4 offset_matrix;
  std::string node_name;
};


struct SceneNode {
  NotNull<ObserverPtr<aiNode const>> ptr;
  std::optional<std::size_t> parent_idx; // Root node has no parent
};


// In preorder.
using SceneNodeList = std::vector<SceneNode>;

using MeshIndex = std::size_t;
using NodeIndex = std::size_t;

using MeshGroupIndex = std::size_t;
using GroupsByNode = std::vector<std::vector<MeshGroupIndex>>;
using AnchorNodeByGroup = std::vector<std::optional<NodeIndex>>;

using SceneMaterialIndex = std::size_t;
using MaterialSlotsByMesh = std::vector<std::vector<SceneMaterialIndex>>;


struct HierarchyLayout {
  SceneNodeList nodes;
  GroupsByNode groups_by_node;
};


struct SubassetLabels {
  std::vector<std::string> keys;
  std::vector<std::string> names;
};
}
