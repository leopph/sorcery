#pragma once

#include <sorcery/animation_data.hpp>
#include <sorcery/assets/Asset.hpp>


namespace sorcery {
class AnimationClip final : public Asset {
  RTTR_ENABLE(Asset)

public:
  AnimationClip() = default;
  AnimationClip(AnimationClip const& other) = delete;
  AnimationClip(AnimationClip&& other) noexcept = delete;
  SORCERYAPI explicit AnimationClip(AnimationClipData&& data);

  ~AnimationClip() override = default;

  auto operator=(AnimationClip const& other) -> AnimationClip& = delete;
  auto operator=(AnimationClip&& other) noexcept -> AnimationClip& = delete;

  SORCERYAPI
  auto SetData(AnimationClipData&& data) -> void;

private:
  std::vector<AnimationChannel> channels_;
  float duration_sec_{0};
};
}
