#include "sorcery/scene/animator_component.hpp"

#include <cmath>
#include <ranges>

#include <spdlog/spdlog.h>

#include "animation_helpers.hpp"
#include "sorcery/timing.hpp"
#include "sorcery/scene/Entity.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::AnimatorComponent>{"Animator Component"}
    .REFLECT_REGISTER_SCENE_OBJECT_CTOR
    .property("clip", &sorcery::AnimatorComponent::GetClip, &sorcery::AnimatorComponent::SetClipRefl);
}


namespace sorcery {
auto AnimatorComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<AnimatorComponent>(*this);
}


auto AnimatorComponent::GetClip() const -> AssetRef<AnimationClip> {
  return clip_;
}


auto AnimatorComponent::SetClip(AssetRef<AnimationClip> const& clip) -> void {
  clip_ = clip;
  needs_path_resolve_ = true;
  time_ = 0.0f;
}


auto AnimatorComponent::Play() -> void {
  if (!is_playing_) {
    time_ = 0.0f;
    is_playing_ = true;
  }
}


auto AnimatorComponent::Stop() -> void {
  is_playing_ = false;
  time_ = 0.0f;
}


auto AnimatorComponent::IsPlaying() const -> bool {
  return is_playing_;
}


auto AnimatorComponent::GetPlaybackTime() const -> float {
  return time_;
}


auto AnimatorComponent::Start() -> void {
  Component::Start();
  Play();
}


auto AnimatorComponent::Update() -> void {
  Component::Update();

  if (!is_playing_) {
    return;
  }

  auto const clip = clip_.Observe();

  if (!clip) {
    return;
  }

  if (clip->GetDuration() < 0) {
    return;
  }

  if (needs_path_resolve_) {
    ResolvePaths(*clip);
    needs_path_resolve_ = false;
  }

  if (clip->GetDuration() == 0) {
    time_ = 0;
  } else {
    time_ = std::fmod(time_ + timing::GetFrameTime(), clip->GetDuration());
  }

  for (auto const [chan_idx, chan] : std::views::enumerate(clip->GetChannels())) {
    auto const chan_transform = transform_by_channel_[chan_idx].Get();

    if (!chan_transform) {
      continue;
    }

    auto const pos = SampleKeys(chan.position_keys, time_, clip->GetDuration(),
      chan_transform->GetLocalPosition(), [](auto const& lhs, auto const& rhs, auto const factor) {
        return Lerp(lhs, rhs, factor);
      });

    auto const rot = SampleKeys(chan.rotation_keys, time_, clip->GetDuration(),
      chan_transform->GetLocalRotation(), [](auto const& lhs, auto const& rhs, auto const factor) {
        return Slerp(lhs, rhs, factor);
      });

    auto const scale = SampleKeys(chan.scaling_keys, time_, clip->GetDuration(),
      chan_transform->GetLocalScale(), [](auto const& lhs, auto const& rhs, auto const factor) {
        return Lerp(lhs, rhs, factor);
      });

    chan_transform->SetLocalTransform(pos, rot, scale);
  }
}


AnimatorComponent::AnimatorComponent() {
  SetUpdatable(true);
}


AnimatorComponent::AnimatorComponent(AnimatorComponent const& other) :
  Component{other},
  clip_{other.clip_} {}


// Reflection requires matching getter and setter types
// ReSharper disable once CppPassValueParameterByConstReference
auto AnimatorComponent::SetClipRefl(AssetRef<AnimationClip> const clip) -> void {
  SetClip(clip);
}


auto AnimatorComponent::ResolvePaths(AnimationClip const& clip) -> void {
  transform_by_channel_.clear();

  auto const channels = clip.GetChannels();
  transform_by_channel_.resize(channels.size());

  for (auto const [chan_idx, chan] : std::views::enumerate(channels)) {
    auto* transform = &GetEntity()->GetTransform();
    auto found_transform = true;

    for (auto const& segment : chan.path) {
      auto occurrence = 0u;
      auto found_child = false;

      for (auto* const child : transform->GetChildren()) {
        if (child->GetEntity()->GetName() == segment.name) {
          if (occurrence == segment.occurrence) {
            transform = child;
            found_child = true;
            break;
          }

          ++occurrence;
        }
      }

      if (!found_child) {
        spdlog::warn(
          R"(Couldn't find Entity "{}" with index "{}" for animation channel "{}" in clip "{}". Skipping channel.)",
          segment.name, segment.occurrence, chan_idx, clip.GetName());
        found_transform = false;
        break;
      }
    }

    if (found_transform) {
      transform_by_channel_[chan_idx] = MakeObjectPtr(MakeObserver(transform));
    }
  }
}
}
