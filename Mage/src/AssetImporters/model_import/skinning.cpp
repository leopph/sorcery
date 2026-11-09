#include "skinning.hpp"

#include <cassert>
#include <memory>
#include <queue>
#include <string_view>

#include <spdlog/spdlog.h>

#include "scene.hpp"


namespace sorcery::mage::model_import {
namespace {
struct Node {
  std::string name;
  Matrix4 transform;
  Node* parent{nullptr};
  std::vector<std::unique_ptr<Node>> children;
  bool visited{false};
};


[[nodiscard]]
auto ConvertHierarchy(aiNode const* node, Node* const converted_parent) -> std::unique_ptr<Node> {
  auto converted_node{
    std::make_unique<Node>(node->mName.C_Str(), Convert(node->mTransformation).Transpose(),
      converted_parent)
  };

  for (unsigned i{0}; i < node->mNumChildren; i++) {
    converted_node->children.emplace_back(ConvertHierarchy(node->mChildren[i], converted_node.get()));
  }

  return converted_node;
}


[[nodiscard]]
auto FindInHierarchyByName(Node& root_node, std::string_view const name) -> Node* {
  if (root_node.name == name) {
    return &root_node;
  }

  for (auto const& child : root_node.children) {
    if (auto const found{FindInHierarchyByName(*child, name)}) {
      return found;
    }
  }

  return nullptr;
}
}


auto ExtractBoneWeights(aiMesh const& mesh, Matrix4 const& transform_inv,
                        std::vector<BoneProcessingInfo>& bone_proc_info, MeshProcessingData& mesh_data) -> void {
  for (unsigned j{0}; j < mesh.mNumBones; j++) {
    auto const bone{mesh.mBones[j]};

    auto const bone_idx{static_cast<std::uint32_t>(bone_proc_info.size())};
    bone_proc_info.emplace_back(transform_inv * Convert(bone->mOffsetMatrix).Transpose(), bone->mName.C_Str());

    for (unsigned k{0}; k < bone->mNumWeights; k++) {
      auto const& weight{bone->mWeights[k]};

      auto found_free_weight_slot{false};

      for (auto l{0}; l < 4; l++) {
        if (mesh_data.bone_weights[weight.mVertexId][l] == 0.0f) {
          mesh_data.bone_weights[weight.mVertexId][l] = weight.mWeight;
          mesh_data.bone_indices[weight.mVertexId][l] = bone_idx;
          found_free_weight_slot = true;
          break;
        }
      }

      assert(found_free_weight_slot);
    }
  }
}


auto BuildSkeleton(aiNode const& root, std::span<BoneProcessingInfo const> const bone_proc_info,
                   std::vector<SkeletonNode>& out_skeleton,
                   std::vector<Bone>& out_bones) -> std::unordered_map<std::string, std::uint32_t> {
  // Convert node hierarchy

  auto root_node{ConvertHierarchy(&root, nullptr)};

  // Mark nodes that are part of the skeleton (but not necessarily bones!)

  std::ranges::for_each(bone_proc_info, [&root_node](BoneProcessingInfo const& bone_info) {
    // We look for the node corresponding to the bone and mark it along with all of its ancestors
    auto node{FindInHierarchyByName(*root_node, bone_info.node_name)};
    assert(node);

    while (node) {
      node->visited = true;
      node = node->parent;
    }
  });

  // Flatten visited hierarchy to a BFS list

  // Indices are needed for node animations. It is guaranteed that animated nodes have unique names.
  std::unordered_map<std::string, std::uint32_t> skeleton_node_name_to_idx;

  struct NodeAndParentIdx {
    Node* node;
    std::optional<std::uint32_t> parent_idx;
  };

  std::queue<NodeAndParentIdx> node_queue;

  if (!root_node->visited) {
    root_node.reset();
  } else {
    node_queue.emplace(root_node.get(), std::nullopt);
  }

  while (!node_queue.empty()) {
    auto const& [node, parent_idx]{node_queue.front()};
    auto const this_node_idx{static_cast<std::uint32_t>(out_skeleton.size())};
    out_skeleton.emplace_back(node->name, node->transform, parent_idx);
    skeleton_node_name_to_idx.try_emplace(node->name, this_node_idx);

    for (auto const& child : node->children) {
      if (child->visited) {
        node_queue.emplace(child.get(), this_node_idx);
      }
    }

    node_queue.pop();
  }

  // Create final bone data

  out_bones.reserve(bone_proc_info.size());
  std::ranges::transform(bone_proc_info, std::back_inserter(out_bones),
    [&skeleton_node_name_to_idx](BoneProcessingInfo const& bone_info) {
      return Bone{
        .offset_mtx = bone_info.offset_matrix, .skeleton_node_idx = skeleton_node_name_to_idx.at(bone_info.node_name)
      };
    });

  return skeleton_node_name_to_idx;
}


auto CollectAnimations(aiScene const& scene,
                       std::unordered_map<std::string, std::uint32_t> const& skeleton_node_name_to_idx) -> std::vector<
  Animation> {
  std::vector<Animation> animations;
  animations.reserve(scene.mNumAnimations);

  for (unsigned i{0}; i < scene.mNumAnimations; i++) {
    auto const anim{scene.mAnimations[i]};

    std::vector<NodeAnimation> node_anims;

    for (unsigned j{0}; j < anim->mNumChannels; j++) {
      auto const channel{anim->mChannels[j]};

      // Check if the animated node is an ancestor of a mesh's node

      std::queue<aiNode const*> node_search_queue;
      node_search_queue.emplace(scene.mRootNode);

      std::queue<aiNode const*> mesh_search_queue;

      // Find the node that the channel is animating
      while (!node_search_queue.empty()) {
        auto const node{node_search_queue.front()};

        if (node->mName == channel->mNodeName) {
          mesh_search_queue.emplace(node);
          break;
        }

        for (unsigned k{0}; k < node->mNumChildren; k++) {
          node_search_queue.emplace(node->mChildren[k]);
        }

        node_search_queue.pop();
      }

      // Check if any of the node's descendants are mesh nodes
      while (!mesh_search_queue.empty()) {
        auto const node{mesh_search_queue.front()};

        if (node->mNumMeshes > 0) {
          spdlog::warn(
            "Animated node is an ancestor of a submesh node. Animations on submesh nodes are currently not supported.");
          break;
        }

        for (unsigned k{0}; k < node->mNumChildren; k++) {
          mesh_search_queue.emplace(node->mChildren[k]);
        }

        mesh_search_queue.pop();
      }

      // Check if the animated node is part of the skeleton

      if (!skeleton_node_name_to_idx.contains(channel->mNodeName.C_Str())) {
        // TODO log this properly
        spdlog::warn("Animated node is not part of the skeleton. Such animations are currently not supported.");
        continue;
      }

      std::vector<AnimPositionKey> position_keys;
      position_keys.reserve(channel->mNumPositionKeys);
      std::ranges::transform(channel->mPositionKeys, channel->mPositionKeys + channel->mNumPositionKeys,
        std::back_inserter(position_keys), [](aiVectorKey const& pos_key) {
          return AnimPositionKey{
            static_cast<float>(pos_key.mTime), Convert(pos_key.mValue)
          };
        });

      std::vector<AnimRotationKey> rotation_keys;
      rotation_keys.reserve(channel->mNumRotationKeys);
      std::ranges::transform(channel->mRotationKeys, channel->mRotationKeys + channel->mNumRotationKeys,
        std::back_inserter(rotation_keys), [](aiQuatKey const& rot_key) {
          return AnimRotationKey{
            static_cast<float>(rot_key.mTime), Convert(rot_key.mValue)
          };
        });

      std::vector<AnimScalingKey> scaling_keys;
      scaling_keys.reserve(channel->mNumScalingKeys);
      std::ranges::transform(channel->mScalingKeys, channel->mScalingKeys + channel->mNumScalingKeys,
        std::back_inserter(scaling_keys), [](aiVectorKey const& scale_key) {
          return AnimScalingKey{
            static_cast<float>(scale_key.mTime), Convert(scale_key.mValue)
          };
        });

      node_anims.emplace_back(std::move(position_keys), std::move(rotation_keys), std::move(scaling_keys),
        skeleton_node_name_to_idx.at(channel->mNodeName.C_Str()));
    }

    animations.emplace_back(anim->mName.C_Str(), static_cast<float>(anim->mDuration),
      static_cast<float>(anim->mTicksPerSecond), std::move(node_anims));
  }

  return animations;
}
}
