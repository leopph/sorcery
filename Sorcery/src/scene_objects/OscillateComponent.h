#pragma once

#include "Component.hpp"
#include "../Math.hpp"


namespace sorcery {
class OscillateComponent final : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

public:
  [[nodiscard]] auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

  auto Start() -> void override;
  auto Update() -> void override;

  LEOPPHAPI OscillateComponent();

private:
  Vector3 direction_{1, 0, 0};
  float speed_{1};
  float distance_{5};
  float cur_dist_{0};
  bool going_backward_{false};
};
}
