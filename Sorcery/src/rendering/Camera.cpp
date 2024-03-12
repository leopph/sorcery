#include "Camera.hpp"


namespace sorcery {
auto Camera::GetNearClipPlane() const noexcept -> float {
  return mNear;
}


auto Camera::SetNearClipPlane(float const near_clip_plane) noexcept -> void {
  if (GetType() == Type::Perspective) {
    mNear = std::max(near_clip_plane, MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE);
    SetFarClipPlane(GetFarClipPlane());
  } else {
    mNear = near_clip_plane;
  }
}


auto Camera::GetFarClipPlane() const noexcept -> float {
  return mFar;
}


auto Camera::SetFarClipPlane(float const far_clip_plane) noexcept -> void {
  if (GetType() == Type::Perspective) {
    mFar = std::max(far_clip_plane, mNear + MINIMUM_PERSPECTIVE_FAR_CLIP_PLANE_OFFSET);
  } else {
    mFar = far_clip_plane;
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


auto Camera::GetRenderTarget() const -> std::shared_ptr<RenderTarget> const& {
  return render_target_;
}


auto Camera::SetRenderTarget(std::shared_ptr<RenderTarget> rt) -> void {
  render_target_ = std::move(rt);
}


auto Camera::CalculateViewMatrix() const noexcept -> Matrix4 {
  return CalculateViewMatrix(GetPosition(), GetRightAxis(), GetUpAxis(), GetForwardAxis());
}


auto Camera::CalculateProjectionMatrix(float const aspect_ratio) const noexcept -> Matrix4 {
  return CalculateProjectionMatrix(GetType(), GetVerticalPerspectiveFov(), GetVerticalOrthographicSize(), aspect_ratio,
    GetNearClipPlane(), GetFarClipPlane());
}


auto Camera::HorizontalPerspectiveFovToVertical(float const fov_degrees, float const aspect_ratio) noexcept -> float {
  return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fov_degrees) / 2.0f) / aspect_ratio));
}


auto Camera::VerticalPerspectiveFovToHorizontal(float const fov_degrees, float const aspect_ratio) noexcept -> float {
  return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fov_degrees) / 2.0f) * aspect_ratio));
}


auto Camera::CalculateViewMatrix(Vector3 const& position, Vector3 const& right, Vector3 const& up,
                                 Vector3 const& forward) -> Matrix4 {
  return Matrix4{
    right[0], up[0], forward[0], 0, right[1], up[1], forward[1], 0, right[2], up[2], forward[2], 0,
    -Dot(position, right), -Dot(position, up), -Dot(position, forward), 1
  };
}


auto Camera::CalculateProjectionMatrix(Type const type, float const fov_deg_vert, float const size_vert,
                                       float const aspect_ratio, float const near_plane,
                                       float const far_plane) -> Matrix4 {
  switch (type) {
    case Type::Perspective: return Matrix4::PerspectiveFov(ToRadians(fov_deg_vert), aspect_ratio, near_plane,
        far_plane);

    case Type::Orthographic: return Matrix4::Orthographic(size_vert * aspect_ratio, size_vert, near_plane, far_plane);
  }

  return Matrix4{};
}
}
