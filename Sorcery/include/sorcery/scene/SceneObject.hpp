#pragma once

#include <sorcery/Object.hpp>


namespace sorcery {
class World;


class SceneObject : public Object {
  RTTR_ENABLE(Object)

protected:
  SceneObject() = default;
  SceneObject(SceneObject const& other) = default;
  SceneObject(SceneObject&& other) noexcept = default;

public:
  ~SceneObject() override = default;

  auto operator=(SceneObject const& other) -> void = delete;
  auto operator=(SceneObject&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI virtual auto Clone() -> std::unique_ptr<SceneObject> = 0;
  LEOPPHAPI virtual auto OnAfterEnteringWorld(World& world) -> void {}
  LEOPPHAPI virtual auto OnBeforeExitingWorld(World& world) -> void {}

  [[nodiscard]] LEOPPHAPI auto IsUpdatable() const -> bool;
  LEOPPHAPI auto SetUpdatable(bool updatable) -> void;

  LEOPPHAPI virtual auto Start() -> void {}
  LEOPPHAPI virtual auto Update() -> void {}

private:
  bool updatable_{false};
};
}
