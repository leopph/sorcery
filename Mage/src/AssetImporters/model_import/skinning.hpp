#pragma once

#include <assimp/mesh.h>

#include "types.hpp"
#include "sorcery/Math.hpp"


namespace sorcery::mage::model_import {
// Appends the bones of the assimp mesh to bones and writes their weights and indices into the mesh
auto ExtractBoneWeights(
  aiMesh const& ai_mesh,
  Matrix4 const& transform_inv,
  std::vector<SourceBone>& bones,
  SourceMesh& mesh
) -> void;


struct BuiltSkeleton {
  std::vector<SkeletonNode> nodes;
  std::vector<Bone> bones;
  std::unordered_map<std::string, std::uint32_t> node_idx_by_name;
};


// Builds the skeleton from the bones' nodes and all of their ancestors, then creates the final bone data.
[[nodiscard]]
auto BuildSkeleton(
  aiNode const& root,
  std::span<SourceBone const> bones
) -> BuiltSkeleton;

// Collects the animations of skeleton nodes. Channels animating other nodes are skipped.
[[nodiscard]]
auto CollectAnimations(
  aiScene const& scene,
  std::unordered_map<std::string, std::uint32_t> const& skeleton_node_name_to_idx
) -> std::vector<Animation>;
}
