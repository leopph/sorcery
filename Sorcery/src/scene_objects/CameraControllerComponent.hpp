#pragma once

#include "Component.hpp"


namespace sorcery {
class CameraControllerComponent final : public Component {
  RTTR_ENABLE(Component)

public:
  [[nodiscard]] auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

  LEOPPHAPI auto Start() -> void override;
  LEOPPHAPI auto Update() -> void override;

  LEOPPHAPI CameraControllerComponent();

  [[nodiscard]] LEOPPHAPI auto get_mouse_sens() const -> float;
  LEOPPHAPI auto set_mouse_sens(float sens) -> void;

  [[nodiscard]] LEOPPHAPI auto get_move_speed() const -> float;
  LEOPPHAPI auto set_move_speed(float speed) -> void;

  [[nodiscard]] LEOPPHAPI auto get_sprint_multiplier() const -> float;
  LEOPPHAPI auto set_sprint_multiplier(float multiplier) -> void;

private:
  float mouse_sens_{0.05f};
  float move_speed_{1.0f};
  float sprint_multiplier_{2.0f};
};
}
