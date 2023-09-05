#pragma once

#include "SceneObject.hpp"


namespace sorcery {
class Entity;


class Component : public SceneObject {
  RTTR_REGISTRATION_FRIEND
  RTTR_ENABLE(SceneObject)
  ObserverPtr<Entity> mEntity{nullptr};

public:
  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity&;
  LEOPPHAPI auto SetEntity(Entity& entity) -> void;

  LEOPPHAPI auto OnDestroy() -> void override;
};
}
