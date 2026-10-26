#pragma once

#include <memory>
#include <span>
#include <vector>

#include <sorcery/assets/NativeAsset.hpp>


namespace sorcery {
class Entity;


class Scene final : public NativeAsset {
  RTTR_ENABLE(NativeAsset)
  RTTR_REGISTRATION_FRIEND

public:
  [[nodiscard]] SORCERYAPI
  auto Serialize() const noexcept -> YAML::Node override;
  SORCERYAPI
  auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void override;

  SORCERYAPI Scene();

  LEOPPHAPI
  auto SetEntities(std::span<Entity const* const> entities) -> void;

  LEOPPHAPI
  auto SetEntityHierarchy(std::span<Entity const* const> root_entities) -> void;

  [[nodiscard]] SORCERYAPI
  auto Instantiate() const -> std::vector<std::unique_ptr<Entity>>;

private:
  auto SaveVersionNumber() -> void;

  YAML::Node yaml_data_;
  YamlDeserializeContext yaml_ctx_;
};
}
