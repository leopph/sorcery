#pragma once

#include <vector>

#include <sorcery/assets/NativeAsset.hpp>


namespace sorcery {
class World;


class Scene final : public NativeAsset {
  RTTR_ENABLE(NativeAsset)
  RTTR_REGISTRATION_FRIEND

public:
  SORCERYAPI Scene();

  SORCERYAPI auto Save(World const& world) -> void;
  SORCERYAPI auto Load(World& world) -> void;

  [[nodiscard]] SORCERYAPI auto Serialize() const noexcept -> YAML::Node override;
  SORCERYAPI auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void override;

private:
  auto SaveVersionNumber() -> void;

  YAML::Node yaml_data_;
  YamlDeserializeContext yaml_ctx_;
};
}
