#pragma once

#include "../Object.hpp"


namespace sorcery {
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

  [[nodiscard]] LEOPPHAPI virtual auto Clone() -> SceneObject* = 0;
};
}
