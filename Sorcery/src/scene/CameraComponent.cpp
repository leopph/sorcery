#include "sorcery/scene/CameraComponent.hpp"

#include "sorcery/reflection.hpp"
#include "sorcery/world.hpp"
#include "sorcery/scene/Entity.hpp"
#include "sorcery/scene/TransformComponent.hpp"


RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::CameraComponent::Type>("Camera Type")(
    rttr::value("Orthographic", sorcery::CameraComponent::Type::Orthographic),
    rttr::value("Perspective", sorcery::CameraComponent::Type::Perspective));

  rttr::registration::class_<sorcery::CameraComponent>{"Camera Component"}
    .REFLECT_REGISTER_SCENE_OBJECT_CTOR
    .property("type", &sorcery::CameraComponent::GetType, &sorcery::CameraComponent::SetType)
    .property("fov", &sorcery::CameraComponent::GetVerticalPerspectiveFov,
      &sorcery::CameraComponent::SetVerticalPerspectiveFov)
    .property("size", &sorcery::CameraComponent::GetVerticalOrthographicSize,
      &sorcery::CameraComponent::SetVerticalOrthographicSize)
    .property("near", &sorcery::CameraComponent::GetNearClipPlane, &sorcery::CameraComponent::SetNearClipPlane)
    .property("far", &sorcery::CameraComponent::GetFarClipPlane, &sorcery::CameraComponent::SetFarClipPlane)
    .property("background", &sorcery::CameraComponent::GetBackgroundColor,
      &sorcery::CameraComponent::SetBackgroundColor)
    .property("viewport", &sorcery::CameraComponent::GetViewport, &sorcery::CameraComponent::SetViewport)
    .property("order", &sorcery::CameraComponent::GetOrder, &sorcery::CameraComponent::SetOrder);
}


namespace sorcery {
auto CameraComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<CameraComponent>(*this);
}


auto CameraComponent::OnAfterEnteringWorld(World& world) -> void {
  Component::OnAfterEnteringWorld(world);
  world.RegisterCamera(*this);
}


auto CameraComponent::OnBeforeExitingWorld(World& world) -> void {
  world.UnregisterCamera(*this);
  Component::OnBeforeExitingWorld(world);
}


auto CameraComponent::GetBackgroundColor() const -> Vector4 const& {
  return background_color_;
}


auto CameraComponent::SetBackgroundColor(Vector4 const& color) -> void {
  for (auto i = 0; i < 4; i++) {
    background_color_[i] = std::clamp(color[i], 0.f, 1.f);
  }
}


auto CameraComponent::GetOrder() const noexcept -> int {
  return order_;
}


auto CameraComponent::SetOrder(int const order) noexcept -> void {
  order_ = order;
}


auto CameraComponent::GetPosition() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetWorldPosition();
}


auto CameraComponent::GetRightAxis() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetRightAxis();
}


auto CameraComponent::GetUpAxis() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetUpAxis();
}


auto CameraComponent::GetForwardAxis() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetForwardAxis();
}
}
