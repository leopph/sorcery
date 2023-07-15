#pragma once

#include "SceneElement.hpp"


namespace sorcery {
class Entity;
class TransformComponent;


class Component : public SceneElement {
  RTTR_ENABLE(SceneElement)
  Entity* mEntity{ nullptr };

public:
  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity*;
  LEOPPHAPI auto SetEntity(Entity* entity) -> void;
};
}
