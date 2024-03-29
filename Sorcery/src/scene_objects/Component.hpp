#pragma once

#include "SceneObject.hpp"


namespace sorcery {
class Entity;


class Component : public SceneObject {
  RTTR_REGISTRATION_FRIEND
  RTTR_ENABLE(SceneObject)
  Entity* mEntity{nullptr};

public:
  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity&;
  LEOPPHAPI auto SetEntity(Entity& entity) -> void;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;
};
}
