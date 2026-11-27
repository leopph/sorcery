#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <sorcery/Math.hpp>


namespace sorcery {
template<typename T>
struct AnimationKey {
  float timestamp;
  T value;
};


using AnimPositionKey = AnimationKey<Vector3>;
using AnimRotationKey = AnimationKey<Quaternion>;
using AnimScalingKey = AnimationKey<Vector3>;


struct AnimationPathSegment {
  std::string name{};
  std::uint32_t occurrence{0};
};


struct AnimationChannel {
  std::vector<AnimationPathSegment> path; // empty = prefab root
  std::vector<AnimPositionKey> position_keys;
  std::vector<AnimRotationKey> rotation_keys;
  std::vector<AnimScalingKey> scaling_keys;
};


struct AnimationClipData {
  float duration{0}; // seconds
  std::vector<AnimationChannel> channels;
};
}
