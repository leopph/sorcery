#include <imgui.h>

#include "ObjectWrappers.hpp"


namespace sorcery::mage {
auto ObjectWrapperFor<CameraComponent>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto& cam{ dynamic_cast<CameraComponent&>(object) };

  ImGui::Text("Type");
  ImGui::TableNextColumn();

  constexpr char const* typeOptions[]{ "Perspective", "Orthographic" };
  int selection{
    cam.GetType() == Camera::Type::Perspective
      ? 0
      : 1
  };
  if (ImGui::Combo("###CameraType", &selection, typeOptions, 2)) {
    cam.SetType(selection == 0
                  ? Camera::Type::Perspective
                  : Camera::Type::Orthographic);
  }

  ImGui::TableNextColumn();

  if (cam.GetType() == Camera::Type::Perspective) {
    ImGui::Text("Field Of View");
    ImGui::TableNextColumn();
    float value{ cam.GetHorizontalPerspectiveFov() };
    if (ImGui::DragFloat("FOV", &value)) {
      cam.SetHorizontalPerspectiveFov(value);
    }
  } else {
    ImGui::Text("Size");
    ImGui::TableNextColumn();
    float value{ cam.GetHorizontalOrthographicSize() };
    if (ImGui::DragFloat("OrthoSize", &value)) {
      cam.SetHorizontalOrthographicSize(value);
    }
  }

  ImGui::TableNextColumn();
  ImGui::Text("Near Clip Plane");
  ImGui::TableNextColumn();

  float nearValue{ cam.GetNearClipPlane() };
  if (ImGui::DragFloat("NearClip", &nearValue)) {
    cam.SetNearClipPlane(nearValue);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Far Clip Plane");
  ImGui::TableNextColumn();

  float farValue{ cam.GetFarClipPlane() };
  if (ImGui::DragFloat("FarClip", &farValue)) {
    cam.SetFarClipPlane(farValue);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Viewport");
  ImGui::TableNextColumn();

  auto viewport{ cam.GetViewport() };
  if (ImGui::InputFloat("ViewportX", &viewport.position.x)) {
    cam.SetViewport(viewport);
  }
  if (ImGui::InputFloat("ViewportY", &viewport.position.y)) {
    cam.SetViewport(viewport);
  }
  if (ImGui::InputFloat("ViewportW", &viewport.extent.width)) {
    cam.SetViewport(viewport);
  }
  if (ImGui::InputFloat("ViewportH", &viewport.extent.height)) {
    cam.SetViewport(viewport);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Background Color");
  ImGui::TableNextColumn();

  Vector4 color{ cam.GetBackgroundColor() };
  if (ImGui::ColorEdit4("###backgroundColor", color.GetData())) {
    cam.SetBackgroundColor(color);
  }
}
}
