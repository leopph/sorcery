#pragma once

#include <assimp/mesh.h>

#include "types.hpp"
#include "sorcery/Math.hpp"


namespace sorcery::mage::model_import {
// Appends the mesh's bones to bone_proc_info and writes their weights and indices into the mesh data
auto ExtractBoneWeights(
  aiMesh const& mesh,
  Matrix4 const& transform_inv,
  std::vector<BoneProcessingInfo>& bone_proc_info,
  MeshProcessingData& mesh_data
) -> void;

// Builds the skeleton from the bones' nodes and all of their ancestors, then creates the final bone data.
// Returns the skeleton node index of each skeleton node name.
[[nodiscard]]
auto BuildSkeleton(
  aiNode const& root,
  std::span<BoneProcessingInfo const> const bone_proc_info,
  std::vector<SkeletonNode>& out_skeleton,
  std::vector<Bone>& out_bones
) -> std::unordered_map<std::string, std::uint32_t>;

// Collects the animations of skeleton nodes. Channels animating other nodes are skipped.
[[nodiscard]]
auto CollectAnimations(
  aiScene const& scene,
  std::unordered_map<std::string, std::uint32_t> const& skeleton_node_name_to_idx
) -> std::vector<Animation>;
}
