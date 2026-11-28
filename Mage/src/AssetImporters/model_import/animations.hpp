#pragma once

#include <vector>

#include <assimp/scene.h>

#include "types.hpp"
#include "sorcery/animation_data.hpp"


namespace sorcery::mage::model_import {
[[nodiscard]]
auto ImportAnimationClips(aiScene const& scene, SceneNodeList const& nodes) -> std::vector<AnimationClipData>;
}
