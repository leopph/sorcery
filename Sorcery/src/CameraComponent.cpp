#include "CameraComponent.hpp"

#include "Entity.hpp"
#include "Renderer.hpp"
#include "TransformComponent.hpp"
#include "Systems.hpp"
#include "Reflection.hpp"

#include <iostream>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::CameraComponent>{ "Camera Component" }
    .REFLECT_REGISTER_SCENE_OBJECT_CTOR
    .property("fov", &sorcery::CameraComponent::GetHorizontalPerspectiveFov, &sorcery::CameraComponent::SetHorizontalPerspectiveFov)
    .property("size", &sorcery::CameraComponent::GetHorizontalOrthographicSize, &sorcery::CameraComponent::SetHorizontalOrthographicSize)
    .property("near", &sorcery::CameraComponent::GetNearClipPlane, &sorcery::CameraComponent::SetNearClipPlane)
    .property("far", &sorcery::CameraComponent::GetFarClipPlane, &sorcery::CameraComponent::SetFarClipPlane)
    .property("background", &sorcery::CameraComponent::GetBackgroundColor, &sorcery::CameraComponent::SetBackgroundColor);
}


namespace sorcery {
Object::Type const CameraComponent::SerializationType{ Object::Type::Camera };


CameraComponent::CameraComponent() {
  gRenderer.RegisterGameCamera(*this);
}


CameraComponent::~CameraComponent() {
  gRenderer.UnregisterGameCamera(*this);
}


auto CameraComponent::GetViewport() const -> NormalizedViewport const& {
  return mViewport;
}


auto CameraComponent::SetViewport(NormalizedViewport const& viewport) -> void {
  mViewport.extent.width = std::clamp(viewport.extent.width, 0.f, 1.f);
  mViewport.extent.height = std::clamp(viewport.extent.height, 0.f, 1.f);
  mViewport.position.x = std::clamp(viewport.position.x, 0.f, 1.f);
  mViewport.position.y = std::clamp(viewport.position.y, 0.f, 1.f);
}


auto CameraComponent::GetBackgroundColor() const -> Vector4 const& {
  return mBackgroundColor;
}


auto CameraComponent::SetBackgroundColor(Vector4 const& color) -> void {
  for (auto i = 0; i < 4; i++) {
    mBackgroundColor[i] = std::clamp(color[i], 0.f, 1.f);
  }
}


auto CameraComponent::GetPosition() const noexcept -> Vector3 {
  return GetEntity().GetTransform().GetWorldPosition();
}


auto CameraComponent::GetRightAxis() const noexcept -> Vector3 {
  return GetEntity().GetTransform().GetRightAxis();
}


auto CameraComponent::GetUpAxis() const noexcept -> Vector3 {
  return GetEntity().GetTransform().GetUpAxis();
}


auto CameraComponent::GetForwardAxis() const noexcept -> Vector3 {
  return GetEntity().GetTransform().GetForwardAxis();
}


auto CameraComponent::GetSerializationType() const -> Object::Type {
  return Object::Type::Camera;
}
}
