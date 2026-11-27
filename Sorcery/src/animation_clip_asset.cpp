#include "sorcery/host/assets/animation_clip_asset.hpp"

#include <utility>

#include "sorcery/serialization.hpp"


namespace sorcery {
auto SerializeAnimationClipData(AnimationClipData const& data) -> std::vector<std::byte> {
  std::vector<std::byte> bytes;

  SerializeToBinary(data.duration, bytes);
  SerializeToBinary(data.channels.size(), bytes);

  for (auto const& channel : data.channels) {
    SerializeToBinary(channel.path.size(), bytes);

    for (auto const& path : channel.path) {
      SerializeToBinary(path.name, bytes);
      SerializeToBinary(path.occurrence, bytes);
    }

    SerializeToBinary(channel.position_keys.size(), bytes);

    for (auto const& key : channel.position_keys) {
      SerializeToBinary(key.timestamp, bytes);

      for (auto i = 0uz; i < 3; ++i) {
        SerializeToBinary(key.value[i], bytes);
      }
    }

    SerializeToBinary(channel.rotation_keys.size(), bytes);

    for (auto const& key : channel.rotation_keys) {
      SerializeToBinary(key.timestamp, bytes);

      SerializeToBinary(key.value.x, bytes);
      SerializeToBinary(key.value.y, bytes);
      SerializeToBinary(key.value.z, bytes);
      SerializeToBinary(key.value.w, bytes);
    }

    SerializeToBinary(channel.scaling_keys.size(), bytes);

    for (auto const& key : channel.scaling_keys) {
      SerializeToBinary(key.timestamp, bytes);

      for (auto i = 0uz; i < 3; ++i) {
        SerializeToBinary(key.value[i], bytes);
      }
    }
  }

  return bytes;
}


auto DeserializeAnimationClipData(
  std::span<std::byte const> const bytes
) -> std::optional<AnimationClipData> {
  auto remaining = auto{bytes};

  auto const read = [&][[nodiscard]](auto& out) -> bool {
    if (!DeserializeFromBinary(remaining, out)) {
      return false;
    }

    remaining = remaining.subspan(GetSerializedSize(out));
    return true;
  };

  AnimationClipData data;

  if (!read(data.duration)) {
    return std::nullopt;
  }

  auto channel_count = 0uz;

  if (!read(channel_count)) {
    return std::nullopt;
  }


  // Channels
  for (auto i = 0uz; i < channel_count; ++i) {
    auto path_count = 0uz;

    if (!read(path_count)) {
      return std::nullopt;
    }

    // Paths
    auto paths = std::vector<AnimationPathSegment>{};

    for (auto j = 0uz; j < path_count; ++j) {
      auto name = std::string{};

      if (!read(name)) {
        return std::nullopt;
      }

      auto occurrence = std::uint32_t{0};

      if (!read(occurrence)) {
        return std::nullopt;
      }

      paths.emplace_back(std::move(name), occurrence);
    }

    auto pos_key_count = 0uz;

    if (!read(pos_key_count)) {
      return std::nullopt;
    }

    // Position keys
    auto pos_keys = std::vector<AnimPositionKey>{};

    for (auto j = 0uz; j < pos_key_count; ++j) {
      auto timestamp = 0.0f;

      if (!read(timestamp)) {
        return std::nullopt;
      }

      auto key = Vector3{};

      for (auto k = 0uz; k < 3; ++k) {
        if (!read(key[k])) {
          return std::nullopt;
        }
      }

      pos_keys.emplace_back(timestamp, key);
    }

    auto rot_key_count = 0uz;

    if (!read(rot_key_count)) {
      return std::nullopt;
    }

    // Rotation keys
    auto rot_keys = std::vector<AnimRotationKey>{};

    for (auto j = 0uz; j < rot_key_count; ++j) {
      auto timestamp = 0.0f;

      if (!read(timestamp)) {
        return std::nullopt;
      }

      auto key = Quaternion{};

      if (!read(key.x)) {
        return std::nullopt;
      }

      if (!read(key.y)) {
        return std::nullopt;
      }

      if (!read(key.z)) {
        return std::nullopt;
      }

      if (!read(key.w)) {
        return std::nullopt;
      }

      rot_keys.emplace_back(timestamp, key);
    }

    auto scale_key_count = 0uz;

    if (!read(scale_key_count)) {
      return std::nullopt;
    }

    // Scale keys
    auto scale_keys = std::vector<AnimScalingKey>{};

    for (auto j = 0uz; j < scale_key_count; ++j) {
      auto timestamp = 0.0f;

      if (!read(timestamp)) {
        return std::nullopt;
      }

      auto key = Vector3{};

      for (auto k = 0uz; k < 3; ++k) {
        if (!read(key[k])) {
          return std::nullopt;
        }
      }

      scale_keys.emplace_back(timestamp, key);
    }

    data.channels.emplace_back(std::move(paths), std::move(pos_keys), std::move(rot_keys), std::move(scale_keys));
  }

  return data;
}
}
