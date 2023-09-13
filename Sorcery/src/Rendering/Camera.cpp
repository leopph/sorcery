#include "Camera.hpp"


namespace sorcery {
auto Camera::GetNearClipPlane() const noexcept -> float {
  return mNear;
}


auto Camera::SetNearClipPlane(float const nearClipPlane) noexcept -> void {
  if (GetType() == Type::Perspective) {
    mNear = std::max(nearClipPlane, MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE);
    SetFarClipPlane(GetFarClipPlane());
  } else {
    mNear = nearClipPlane;
  }
}


auto Camera::GetFarClipPlane() const noexcept -> float {
  return mFar;
}


auto Camera::SetFarClipPlane(float const farClipPlane) noexcept -> void {
  if (GetType() == Type::Perspective) {
    mFar = std::max(farClipPlane, mNear + MINIMUM_PERSPECTIVE_FAR_CLIP_PLANE_OFFSET);
  } else {
    mFar = farClipPlane;
  }
}


auto Camera::GetType() const noexcept -> Type {
  return mType;
}


auto Camera::SetType(Type const type) noexcept -> void {
  mType = type;

  if (type == Type::Perspective) {
    SetNearClipPlane(GetNearClipPlane());
  }
}


auto Camera::GetHorizontalPerspectiveFov() const -> float {
  return mPerspFovHorizDeg;
}


auto Camera::SetHorizontalPerspectiveFov(float degrees) -> void {
  degrees = std::max(degrees, MINIMUM_PERSPECTIVE_HORIZONTAL_FOV);
  mPerspFovHorizDeg = degrees;
}


auto Camera::GetHorizontalOrthographicSize() const -> float {
  return mOrthoSizeHoriz;
}


auto Camera::SetHorizontalOrthographicSize(float size) -> void {
  size = std::max(size, MINIMUM_ORTHOGRAPHIC_HORIZONTAL_SIZE);
  mOrthoSizeHoriz = size;
}


auto Camera::CalculateViewMatrix() const noexcept -> Matrix4 {
  return Matrix4::LookTo(GetPosition(), GetForwardAxis(), Vector3::Up());
}


auto Camera::CalculateProjectionMatrix(float const aspectRatio) const noexcept -> Matrix4 {
  switch (GetType()) {
    case Type::Perspective:
      return Matrix4::PerspectiveFov(
        ToRadians(Camera::HorizontalPerspectiveFovToVertical(GetHorizontalPerspectiveFov(), aspectRatio)), aspectRatio,
        GetNearClipPlane(), GetFarClipPlane());

    case Type::Orthographic:
      return Matrix4::Orthographic(GetHorizontalOrthographicSize(),
        GetHorizontalOrthographicSize() / aspectRatio, GetNearClipPlane(), GetFarClipPlane());
  }

  return Matrix4{};
}


auto Camera::HorizontalPerspectiveFovToVertical(float const fovDegrees, float const aspectRatio) noexcept -> float {
  return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fovDegrees) / 2.0f) / aspectRatio));
}


auto Camera::VerticalPerspectiveFovToHorizontal(float const fovDegrees, float const aspectRatio) noexcept -> float {
  return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fovDegrees) / 2.0f) * aspectRatio));
}
}
