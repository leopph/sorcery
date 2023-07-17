#pragma once

#include "SceneObject.hpp"


namespace sorcery {
class Entity;
class TransformComponent;


class Component : public SceneObject {
  RTTR_ENABLE(SceneObject)
  ObserverPtr<Entity> mEntity{ nullptr };

public:
  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity&;
  LEOPPHAPI auto SetEntity(Entity& entity) -> void;
};
}
