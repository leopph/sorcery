#pragma once

#include <vector>

#include <sorcery/asset_ref.hpp>
#include <sorcery/object_ptr.hpp>
#include <sorcery/assets/animation_clip.hpp>
#include <sorcery/scene/Component.hpp>


namespace sorcery {
class TransformComponent;


class AnimatorComponent : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

public:
  [[nodiscard]] SORCERYAPI
  auto Clone() -> std::unique_ptr<SceneObject> override;

  [[nodiscard]] SORCERYAPI
  auto GetClip() const -> AssetRef<AnimationClip>;
  SORCERYAPI
  auto SetClip(AssetRef<AnimationClip> const& clip) -> void;

  SORCERYAPI
  auto Play() -> void;

  SORCERYAPI
  auto Stop() -> void;

  [[nodiscard]] SORCERYAPI
  auto IsPlaying() const -> bool;

  [[nodiscard]] SORCERYAPI
  auto GetPlaybackTime() const -> float;

  SORCERYAPI
  auto Start() -> void override;

  SORCERYAPI
  auto Update() -> void override;

  SORCERYAPI
  AnimatorComponent();

  SORCERYAPI
  AnimatorComponent(AnimatorComponent const& other);
  AnimatorComponent(AnimatorComponent&& other) noexcept = delete;

  ~AnimatorComponent() override = default;

  auto operator=(AnimatorComponent const& other) -> AnimatorComponent& = delete;
  auto operator=(AnimatorComponent&& other) -> AnimatorComponent& = delete;

private:
  auto SetClipRefl(AssetRef<AnimationClip> clip) -> void;
  auto ResolvePaths(AnimationClip const& clip) -> void;

  AssetRef<AnimationClip> clip_;
  bool is_playing_{false};
  float time_{0};
  bool needs_path_resolve_{true};
  std::vector<ObjectPtr<TransformComponent>> transform_by_channel_;
};
}
