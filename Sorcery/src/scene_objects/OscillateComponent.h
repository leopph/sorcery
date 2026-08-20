#pragma once

#include "Component.hpp"
#include "../Math.hpp"


namespace sorcery {
class OscillateComponent final : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

public:
  [[nodiscard]] auto Clone() -> std::unique_ptr<SceneObject> override;

  auto Start() -> void override;
  auto Update() -> void override;

  LEOPPHAPI OscillateComponent();

  [[nodiscard]] SORCERYAPI
  auto GetDirection() const -> Vector3 const&;

  SORCERYAPI
  auto SetDirection(Vector3 const& dir) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetSpeed() const -> float;

  SORCERYAPI
  auto SetSpeed(float speed) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetDistance() const -> float;

  SORCERYAPI
  auto SetDistance(float dist) -> void;

private:
  Vector3 direction_{1, 0, 0};
  float speed_{1};
  float distance_{5};
  float cur_dist_{0};
  bool going_backward_{false};
};
}
