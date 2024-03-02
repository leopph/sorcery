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


auto Camera::GetVerticalPerspectiveFov() const -> float {
  return mVertPerspFovDeg;
}


auto Camera::SetVerticalPerspectiveFov(float degrees) -> void {
  degrees = std::max(degrees, MINIMUM_PERSPECTIVE_VERTICAL_FOV);
  mVertPerspFovDeg = degrees;
}


auto Camera::GetVerticalOrthographicSize() const -> float {
  return mVertOrhoSize;
}


auto Camera::SetVerticalOrthographicSize(float size) -> void {
  size = std::max(size, MINIMUM_ORTHOGRAPHIC_VERTICAL_SIZE);
  mVertOrhoSize = size;
}


auto Camera::CalculateViewMatrix() const noexcept -> Matrix4 {
  auto const rightAxis{GetRightAxis()};
  auto const upAxis{GetUpAxis()};
  auto const forwardAxis{GetForwardAxis()};
  auto const position{GetPosition()};

  return Matrix4{
    rightAxis[0], upAxis[0], forwardAxis[0], 0,
    rightAxis[1], upAxis[1], forwardAxis[1], 0,
    rightAxis[2], upAxis[2], forwardAxis[2], 0,
    -Dot(position, rightAxis), -Dot(position, upAxis), -Dot(position, forwardAxis), 1
  };
}


auto Camera::CalculateProjectionMatrix(float const aspectRatio) const noexcept -> Matrix4 {
  switch (GetType()) {
    case Type::Perspective:
      return Matrix4::PerspectiveFov(ToRadians(GetVerticalPerspectiveFov()), aspectRatio, GetNearClipPlane(), GetFarClipPlane());

    case Type::Orthographic:
      return Matrix4::Orthographic(GetVerticalOrthographicSize() * aspectRatio, GetVerticalOrthographicSize(), GetNearClipPlane(), GetFarClipPlane());
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
