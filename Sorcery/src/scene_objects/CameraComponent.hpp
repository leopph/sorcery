#pragma once

#include "Component.hpp"
#include "../rendering/Camera.hpp"
#include "../Math.hpp"


namespace sorcery {
class CameraComponent final : public Component, public rendering::Camera {
  RTTR_ENABLE(Component)

public:
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnAfterEnteringScene(Scene const& scene) -> void override;
  LEOPPHAPI auto OnBeforeExitingScene(Scene const& scene) -> void override;

  [[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const -> Vector4 const&;
  LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;

  [[nodiscard]] auto LEOPPHAPI GetPosition() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetRightAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetUpAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetForwardAxis() const noexcept -> Vector3 override;

private:
  Vector4 background_color_{0, 0, 0, 1};
};
}
