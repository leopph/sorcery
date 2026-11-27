#include "sorcery/assets/animation_clip.hpp"

#include <utility>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::AnimationClip>{"AnimationClip"};
}


namespace sorcery {
AnimationClip::AnimationClip(AnimationClipData&& data) {
  SetData(std::move(data));
}


auto AnimationClip::SetData(AnimationClipData&& data) -> void {
  channels_ = std::move(data.channels);
  duration_sec_ = data.duration;
}
}
