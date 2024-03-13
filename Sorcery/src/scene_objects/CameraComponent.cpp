#include "CameraComponent.hpp"

#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "../engine_context.hpp"
#include "../Reflection.hpp"

#include <imgui.h>

#include <iostream>


RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::CameraComponent::Type>("Camera Type")(
    rttr::value("Orthographic", sorcery::CameraComponent::Type::Orthographic),
    rttr::value("Perspective", sorcery::CameraComponent::Type::Perspective)
  );

  rttr::registration::class_<sorcery::CameraComponent>{"Camera Component"}
    .REFLECT_REGISTER_SCENE_OBJECT_CTOR
    .property("fov", &sorcery::CameraComponent::GetVerticalPerspectiveFov, &sorcery::CameraComponent::SetVerticalPerspectiveFov)
    .property("size", &sorcery::CameraComponent::GetVerticalOrthographicSize, &sorcery::CameraComponent::SetVerticalOrthographicSize)
    .property("near", &sorcery::CameraComponent::GetNearClipPlane, &sorcery::CameraComponent::SetNearClipPlane)
    .property("far", &sorcery::CameraComponent::GetFarClipPlane, &sorcery::CameraComponent::SetFarClipPlane)
    .property("background", &sorcery::CameraComponent::GetBackgroundColor, &sorcery::CameraComponent::SetBackgroundColor);
}


namespace sorcery {
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


auto CameraComponent::OnInit() -> void {
  Component::OnInit();
  g_engine_context.scene_renderer->Register(*this);
}


auto CameraComponent::OnDestroy() -> void {
  g_engine_context.scene_renderer->Unregister(*this);
  Component::OnDestroy();
}


auto CameraComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("Type");
  ImGui::TableNextColumn();

  constexpr char const* typeOptions[]{"Perspective", "Orthographic"};
  int selection{
    GetType() == Camera::Type::Perspective
      ? 0
      : 1
  };
  if (ImGui::Combo("###CameraType", &selection, typeOptions, 2)) {
    SetType(selection == 0
              ? Camera::Type::Perspective
              : Camera::Type::Orthographic);
  }

  ImGui::TableNextColumn();

  if (GetType() == Camera::Type::Perspective) {
    ImGui::Text("Field Of View");
    ImGui::TableNextColumn();
    float value{GetVerticalPerspectiveFov()};
    if (ImGui::DragFloat("FOV", &value)) {
      SetVerticalPerspectiveFov(value);
    }
  } else {
    ImGui::Text("Size");
    ImGui::TableNextColumn();
    float value{GetVerticalOrthographicSize()};
    if (ImGui::DragFloat("OrthoSize", &value)) {
      SetVerticalOrthographicSize(value);
    }
  }

  ImGui::TableNextColumn();
  ImGui::Text("Near Clip Plane");
  ImGui::TableNextColumn();

  float nearValue{GetNearClipPlane()};
  if (ImGui::DragFloat("NearClip", &nearValue)) {
    SetNearClipPlane(nearValue);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Far Clip Plane");
  ImGui::TableNextColumn();

  float farValue{GetFarClipPlane()};
  if (ImGui::DragFloat("FarClip", &farValue)) {
    SetFarClipPlane(farValue);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Viewport");
  ImGui::TableNextColumn();

  auto viewport{GetViewport()};
  if (ImGui::InputFloat("ViewportX", &viewport.position.x)) {
    SetViewport(viewport);
  }
  if (ImGui::InputFloat("ViewportY", &viewport.position.y)) {
    SetViewport(viewport);
  }
  if (ImGui::InputFloat("ViewportW", &viewport.extent.width)) {
    SetViewport(viewport);
  }
  if (ImGui::InputFloat("ViewportH", &viewport.extent.height)) {
    SetViewport(viewport);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Background Color");
  ImGui::TableNextColumn();

  Vector4 color{GetBackgroundColor()};
  if (ImGui::ColorEdit4("###backgroundColor", color.GetData())) {
    SetBackgroundColor(color);
  }
}
}
