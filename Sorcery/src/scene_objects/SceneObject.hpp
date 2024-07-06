#pragma once

#include "../Object.hpp"


namespace sorcery {
class Scene;


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
  LEOPPHAPI virtual auto OnAfterEnteringScene(Scene const& scene) -> void {}
  LEOPPHAPI virtual auto OnBeforeExitingScene(Scene const& scene) -> void {}

  [[nodiscard]] LEOPPHAPI auto IsUpdatable() const -> bool;
  LEOPPHAPI auto SetUpdatable(bool updatable) -> void;

  LEOPPHAPI virtual auto Update() -> void {}

private:
  bool updatable_{false};
};
}
