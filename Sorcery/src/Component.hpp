#pragma once

#include "SceneObject.hpp"


namespace sorcery {
class Entity;


class Component : public SceneObject {
  RTTR_REGISTRATION_FRIEND
  RTTR_ENABLE(SceneObject)
  ObserverPtr<Entity> mEntity{nullptr};

public:
  Component() = default;
  Component(Component const&) = delete;
  Component(Component&&) = delete;

  LEOPPHAPI ~Component() override;

  void operator=(Component const&) = delete;
  void operator=(Component&&) = delete;

  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity&;
  LEOPPHAPI auto SetEntity(Entity& entity) -> void;
};
}
