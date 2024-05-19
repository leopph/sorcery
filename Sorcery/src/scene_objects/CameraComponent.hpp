#pragma once

#include "Component.hpp"
#include "../rendering/Camera.hpp"

#include "../Math.hpp"
#include "../Util.hpp"


namespace sorcery {
class CameraComponent final : public Component, public rendering::Camera {
  RTTR_ENABLE(Component)

public:
  [[nodiscard]] LEOPPHAPI auto Clone() -> CameraComponent* override;

  [[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const -> Vector4 const&;
  LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;

  [[nodiscard]] auto LEOPPHAPI GetPosition() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetRightAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetUpAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetForwardAxis() const noexcept -> Vector3 override;

  LEOPPHAPI auto OnInit() -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

private:
  Vector4 mBackgroundColor{0, 0, 0, 1};
};
}
