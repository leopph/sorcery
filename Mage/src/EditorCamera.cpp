#include "EditorCamera.hpp"


namespace sorcery::mage {
auto EditorCamera::GetPosition() const noexcept -> Vector3 {
  return position;
}


auto EditorCamera::GetRightAxis() const noexcept -> Vector3 {
  return orientation.Rotate(Vector3::Right());
}


auto EditorCamera::GetUpAxis() const noexcept -> Vector3 {
  return orientation.Rotate(Vector3::Up());
}


auto EditorCamera::GetForwardAxis() const noexcept -> Vector3 {
  return orientation.Rotate(Vector3::Forward());
}


EditorCamera::EditorCamera(Vector3 const& position, Quaternion const& orientation, float const nearClip, float const farClip, float const fovHorizDeg) :
  position{ position },
  orientation{ orientation } {
  SetNearClipPlane(nearClip);
  SetFarClipPlane(farClip);
  SetHorizontalPerspectiveFov(fovHorizDeg);
}
}
