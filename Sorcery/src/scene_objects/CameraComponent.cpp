#include "CameraComponent.hpp"

#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "../app.hpp"
#include "../Reflection.hpp"
#include "../rendering/scene_renderer.hpp"


RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::CameraComponent::Type>("Camera Type")(
    rttr::value("Orthographic", sorcery::CameraComponent::Type::Orthographic),
    rttr::value("Perspective", sorcery::CameraComponent::Type::Perspective));

  rttr::registration::class_<sorcery::CameraComponent>{"Camera Component"}.REFLECT_REGISTER_SCENE_OBJECT_CTOR.
    property("fov", &sorcery::CameraComponent::GetVerticalPerspectiveFov,
      &sorcery::CameraComponent::SetVerticalPerspectiveFov).property("size",
      &sorcery::CameraComponent::GetVerticalOrthographicSize,
      &sorcery::CameraComponent::SetVerticalOrthographicSize).property("near",
      &sorcery::CameraComponent::GetNearClipPlane, &sorcery::CameraComponent::SetNearClipPlane).property("far",
      &sorcery::CameraComponent::GetFarClipPlane, &sorcery::CameraComponent::SetFarClipPlane).property("background",
      &sorcery::CameraComponent::GetBackgroundColor, &sorcery::CameraComponent::SetBackgroundColor).property("viewport",
      &sorcery::CameraComponent::GetViewport, &sorcery::CameraComponent::SetViewport);
}


namespace sorcery {
auto CameraComponent::Clone() -> std::unique_ptr<SceneObject> {
  return Create<CameraComponent>(*this);
}


auto CameraComponent::OnAfterEnteringScene(Scene const& scene) -> void {
  Component::OnAfterEnteringScene(scene);
  App::Instance().GetSceneRenderer().Register(*this);
}


auto CameraComponent::OnBeforeExitingScene(Scene const& scene) -> void {
  App::Instance().GetSceneRenderer().Unregister(*this);
  Component::OnBeforeExitingScene(scene);
}


auto CameraComponent::GetBackgroundColor() const -> Vector4 const& {
  return background_color_;
}


auto CameraComponent::SetBackgroundColor(Vector4 const& color) -> void {
  for (auto i = 0; i < 4; i++) {
    background_color_[i] = std::clamp(color[i], 0.f, 1.f);
  }
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
