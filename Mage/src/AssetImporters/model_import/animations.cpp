#include "animations.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <string_view>
#include <unordered_map>

#include <spdlog/spdlog.h>

#include "scene.hpp"


namespace sorcery::mage::model_import {
auto ImportAnimationClips(aiScene const& scene, SceneNodeList const& nodes) -> std::vector<AnimationClipData> {
  auto node_idx_by_raw_name = std::unordered_map<std::string_view, std::size_t>{};

  for (auto const [node_idx, node] : std::views::enumerate(nodes)) {
    // The raw node name might not be unique, animations only refer to nodes with unique names,
    // so such nodes are unused.
    node_idx_by_raw_name[node.ptr->mName.C_Str()] = node_idx;
  }

  auto anims = std::vector<AnimationClipData>{};

  for (auto i = 0u; i < scene.mNumAnimations; ++i) {
    auto const* const ai_anim = scene.mAnimations[i];
    auto const ticks_per_sec = ai_anim->mTicksPerSecond != 0.0 ? ai_anim->mTicksPerSecond : 25.0;

    std::vector<AnimationChannel> channels;

    for (auto j = 0u; j < ai_anim->mNumChannels; ++j) {
      auto const* const ai_chan = ai_anim->mChannels[j];
      auto const node_idx_it = node_idx_by_raw_name.find(ai_chan->mNodeName.C_Str());

      if (node_idx_it == node_idx_by_raw_name.end()) {
        spdlog::warn(R"(Animation "{}": channel refers to nonexistent node "{}". Skipping channel.)",
          ai_anim->mName.C_Str(), ai_chan->mNodeName.C_Str());
        continue;
      }

      auto chan = AnimationChannel{};

      // Don't include the root in the path
      for (auto const* node = &nodes[node_idx_it->second]; node->parent_idx; node = &nodes[*node->parent_idx]) {
        chan.path.emplace_back(node->name, node->occurrence);
      }

      std::ranges::reverse(chan.path);

      chan.position_keys.reserve(ai_chan->mNumPositionKeys);
      chan.rotation_keys.reserve(ai_chan->mNumRotationKeys);
      chan.scaling_keys.reserve(ai_chan->mNumScalingKeys);

      auto const convert_vec_key = [&](aiVectorKey const& key) {
        return AnimationKey{
          .timestamp = static_cast<float>(key.mTime / ticks_per_sec),
          .value = Convert(key.mValue)
        };
      };

      auto const convert_quat_key = [&](aiQuatKey const& key) {
        return AnimRotationKey{
          .timestamp = static_cast<float>(key.mTime / ticks_per_sec),
          .value = Convert(key.mValue)
        };
      };

      std::ranges::transform(std::span{ai_chan->mPositionKeys, ai_chan->mNumPositionKeys},
        std::back_inserter(chan.position_keys), convert_vec_key);

      std::ranges::transform(std::span{ai_chan->mRotationKeys, ai_chan->mNumRotationKeys},
        std::back_inserter(chan.rotation_keys), convert_quat_key);

      std::ranges::transform(std::span{ai_chan->mScalingKeys, ai_chan->mNumScalingKeys},
        std::back_inserter(chan.scaling_keys), convert_vec_key);

      channels.emplace_back(std::move(chan));
    }

    auto const duration_sec = ai_anim->mDuration / ticks_per_sec;
    anims.emplace_back(static_cast<float>(duration_sec), std::move(channels));
  }

  return anims;
}
}
