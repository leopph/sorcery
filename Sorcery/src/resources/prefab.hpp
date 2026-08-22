#pragma once

#include "NativeResource.hpp"
#include "../entity_serialization.hpp"


namespace sorcery {
class Entity;


class Prefab : public NativeResource {
  RTTR_ENABLE(NativeResource)

public:
  [[nodiscard]] auto Serialize() const noexcept -> YAML::Node override;
  auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void override;

  Prefab() = default;
  Prefab(Prefab const& other) = delete;
  Prefab(Prefab&& other) = delete;

  ~Prefab() override = default;

  auto operator=(Prefab const& other) -> void = delete;
  auto operator=(Prefab&& other) -> void = delete;

  /**
   * Create a prefab out of the passed entities
   * This function takes ALL of the entities in the sub-hierarchy, not just the roots!
   * Passing only the roots will result in undefined behavior.
   * For a helper function taking only the hierarchy root, see CaptureEntityHierarchy.
   */
  SORCERYAPI
  auto CaptureEntitySet(
    std::span<Entity const* const> entities,
    EntitySerializationContext const& ctx
  ) -> void;

  /**
   * Creates a prefab out of the passed entity sub-hierarchy.
   * This function takes ONLY the roots the sub-hierarchy
   * and discovers the descendant's automatically.
   * For a version taking all of the entities, see CaptureEntitySet.
   */
  SORCERYAPI
  auto CaptureEntityHierarchy(
    std::span<Entity const* const> roots,
    EntitySerializationContext const& ctx
  ) -> void;

  /**
   * Create instances of the sub-hierarchy stored in the prefab.
   * Returns a flat list of all the entities in the sub-hierarchy
   * with parent-child relationships already set.
   */
  [[nodiscard]] SORCERYAPI
  auto Instantiate() const -> std::vector<std::unique_ptr<Entity>>;

private:
  YAML::Node node_;
  YamlDeserializeContext ctx_{.current_guid = Guid::Invalid()};
};
}
