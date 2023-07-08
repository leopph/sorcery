#pragma once

#include <Renderer.hpp>


namespace sorcery::mage {
struct EditorCamera : Camera {
  [[nodiscard]] auto GetPosition() const noexcept -> Vector3 override;
  [[nodiscard]] auto GetRightAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto GetUpAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto GetForwardAxis() const noexcept -> Vector3 override;

  EditorCamera(Vector3 const& position, Quaternion const& orientation, float nearClip, float farClip, float fovHorizDeg);

  Vector3 position;
  Quaternion orientation;
};
}
