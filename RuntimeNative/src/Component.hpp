#pragma once

#include "SceneElement.hpp"


namespace leopph {
class Entity;
class TransformComponent;


class Component : public SceneElement {
  RTTR_ENABLE(SceneElement)
  Entity* mEntity{ nullptr };

public:
  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity*;
  LEOPPHAPI auto SetEntity(Entity* entity) -> void;

  LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override = 0;
  LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override = 0;
};
}
