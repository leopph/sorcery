#pragma once

#include <sorcery/observer_ptr.hpp>
#include <sorcery/scene/SceneObject.hpp>


namespace sorcery {
class Entity;


class Component : public SceneObject {
  RTTR_REGISTRATION_FRIEND
  RTTR_ENABLE(SceneObject)

protected:
  Component() = default;
  Component(Component const& other) = default;
  Component(Component&& other) noexcept = delete;

public:
  ~Component() override = default;

  auto operator=(Component const& other) -> void = delete;
  auto operator=(Component&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetEntity() const -> ObserverPtr<Entity>;

private:
  ObserverPtr<Entity> entity_{nullptr};

  friend class Entity;
};
}
