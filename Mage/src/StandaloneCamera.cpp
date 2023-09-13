#include "StandaloneCamera.hpp"


namespace sorcery::mage {
auto StandaloneCamera::GetPosition() const noexcept -> Vector3 {
  return position;
}


auto StandaloneCamera::GetRightAxis() const noexcept -> Vector3 {
  return orientation.Rotate(Vector3::Right());
}


auto StandaloneCamera::GetUpAxis() const noexcept -> Vector3 {
  return orientation.Rotate(Vector3::Up());
}


auto StandaloneCamera::GetForwardAxis() const noexcept -> Vector3 {
  return orientation.Rotate(Vector3::Forward());
}


StandaloneCamera::StandaloneCamera(Vector3 const& position, Quaternion const& orientation, float const speed, float const nearClip, float const farClip, float const fovHorizDeg) :
  position{position},
  orientation{orientation},
  speed{speed} {
  SetNearClipPlane(nearClip);
  SetFarClipPlane(farClip);
  SetHorizontalPerspectiveFov(fovHorizDeg);
}
}
