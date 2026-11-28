#pragma once

#include <cassert>
#include <cstddef>
#include <optional>
#include <span>
#include <type_traits>

#include "sorcery/animation_data.hpp"


namespace sorcery {
template<typename T, typename Interpolate>
[[nodiscard]]
auto SampleKeys(
  std::span<AnimationKey<std::type_identity_t<T>> const> const keys,
  float const time,
  float const duration,
  T const& fallback,
  Interpolate interpolate
) -> T {
  auto const calc_interpolation_factor = [](float const from_time, float const to_time, float const current_time) {
    return (current_time - from_time) / (to_time - from_time);
  };

  if (keys.empty()) {
    return fallback;
  }

  if (keys.size() == 1) {
    return keys[0].value;
  }

  std::optional<std::size_t> next_key_idx;

  for (auto i = 0uz; i < keys.size(); ++i) {
    if (keys[i].timestamp > time) {
      next_key_idx = i;
      break;
    }
  }

  // If we're after the last or before the first key, interpolate with a wraparound
  if (!next_key_idx || next_key_idx == 0) {
    auto const& first_key = keys.front();
    auto const& last_key = keys.back();

    auto const from_time = last_key.timestamp;
    auto const to_time = first_key.timestamp + duration;
    assert(to_time > from_time);

    auto const extended_time = next_key_idx == 0 ? duration + time : time;
    auto const factor = (extended_time - from_time) / (to_time - from_time);
    return interpolate(last_key.value, first_key.value, factor);
  }

  auto const& next_key = keys[*next_key_idx];
  auto const& prev_key = keys[*next_key_idx - 1];

  return interpolate(prev_key.value, next_key.value,
    calc_interpolation_factor(prev_key.timestamp, next_key.timestamp, time));
}
}
