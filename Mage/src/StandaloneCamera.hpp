#pragma once

#include <..\..\Sorcery\src\rendering\scene_renderer.hpp>


namespace sorcery::mage {
class StandaloneCamera : public Camera {
public:
  [[nodiscard]] auto GetPosition() const noexcept -> Vector3 override;
  [[nodiscard]] auto GetRightAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto GetUpAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto GetForwardAxis() const noexcept -> Vector3 override;

  StandaloneCamera(Vector3 const& position, Quaternion const& orientation, float speed, float nearClip, float farClip, float vertFovDeg);

  Vector3 position;
  Quaternion orientation;
  float speed;
};
}
