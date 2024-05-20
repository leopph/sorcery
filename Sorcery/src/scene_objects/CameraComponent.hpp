#pragma once

#include "Component.hpp"
#include "../rendering/Camera.hpp"

#include "../Math.hpp"
#include "../Util.hpp"


namespace sorcery {
class CameraComponent final : public Component, public rendering::Camera {
  RTTR_ENABLE(Component)

public:
  CameraComponent() = default;
  CameraComponent(CameraComponent const& other) = default;
  CameraComponent(CameraComponent&& other) noexcept = default;

  LEOPPHAPI ~CameraComponent() override;

  auto operator=(CameraComponent const& other) -> CameraComponent& = delete;
  auto operator=(CameraComponent&& other) -> CameraComponent& = delete;

  [[nodiscard]] LEOPPHAPI auto Clone() -> CameraComponent* override;

  [[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const -> Vector4 const&;
  LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;

  [[nodiscard]] auto LEOPPHAPI GetPosition() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetRightAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetUpAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetForwardAxis() const noexcept -> Vector3 override;

  LEOPPHAPI auto Initialize() -> void override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

private:
  Vector4 mBackgroundColor{0, 0, 0, 1};
};
}
