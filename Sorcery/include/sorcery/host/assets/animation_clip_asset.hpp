#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

#include <sorcery/animation_data.hpp>
#include <sorcery/Core.hpp>


namespace sorcery {
[[nodiscard]] SORCERYAPI
auto SerializeAnimationClipData(
  AnimationClipData const& data
) -> std::vector<std::byte>;

[[nodiscard]] SORCERYAPI
auto DeserializeAnimationClipData(
  std::span<std::byte const> bytes
) -> std::optional<AnimationClipData>;
}
