#pragma once

#include "SceneObject.hpp"
#include "../observer_ptr.hpp"


namespace sorcery {
class Entity;


class Component : public SceneObject {
  RTTR_REGISTRATION_FRIEND
  RTTR_ENABLE(SceneObject)

public:
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

  LEOPPHAPI virtual auto OnAfterAttachedToEntity(Entity& entity) -> void;
  LEOPPHAPI virtual auto OnBeforeDetachedFromEntity(Entity& entity) -> void;

protected:
  Component() = default;
  Component(Component const& other) = default;
  Component(Component&& other) noexcept = default;

public:
  ~Component() override = default;

  auto operator=(Component const& other) -> void = delete;
  auto operator=(Component&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> ObserverPtr<Entity>;

private:
  ObserverPtr<Entity> entity_{nullptr};
};
}
