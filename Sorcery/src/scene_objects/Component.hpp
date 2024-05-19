#pragma once

#include "SceneObject.hpp"


namespace sorcery {
class Entity;


class Component : public SceneObject {
  RTTR_REGISTRATION_FRIEND
  RTTR_ENABLE(SceneObject)

protected:
  Component() = default;
  Component(Component const& other) = default;
  Component(Component&& other) noexcept = default;

public:
  ~Component() override = default;

  auto operator=(Component const& other) -> void = delete;
  auto operator=(Component&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto Clone() -> Component* override = 0;

  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity&;
  LEOPPHAPI auto SetEntity(Entity& entity) -> void;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;

private:
  Entity* mEntity{nullptr};
};
}
