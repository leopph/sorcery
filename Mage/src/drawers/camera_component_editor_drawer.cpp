#include "camera_component_editor_drawer.hpp"

#include "editor_drawer_registry.hpp"
#include "gui_helpers.hpp"


namespace sorcery::mage {
auto CameraComponentEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  CameraComponent& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::Text("Type");
  ImGui::TableNextColumn();

  constexpr char const* typeOptions[]{"Perspective", "Orthographic"};
  int selection{obj.GetType() == CameraComponent::Type::Perspective ? 0 : 1};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::Combo("###CameraType", &selection, typeOptions, 2);
  })) {
    obj.SetType(selection == 0 ? CameraComponent::Type::Perspective : CameraComponent::Type::Orthographic);
  }

  ImGui::TableNextColumn();

  if (obj.GetType() == CameraComponent::Type::Perspective) {
    ImGui::Text("Field Of View");
    ImGui::TableNextColumn();
    float value{obj.GetVerticalPerspectiveFov()};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::DragFloat("FOV", &value);
    })) {
      obj.SetVerticalPerspectiveFov(value);
    }
  } else {
    ImGui::Text("Size");
    ImGui::TableNextColumn();
    float value{obj.GetVerticalOrthographicSize()};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::DragFloat("OrthoSize", &value);
    })) {
      obj.SetVerticalOrthographicSize(value);
    }
  }

  ImGui::TableNextColumn();
  ImGui::Text("Near Clip Plane");
  ImGui::TableNextColumn();

  float nearValue{obj.GetNearClipPlane()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("NearClip", &nearValue);
  })) {
    obj.SetNearClipPlane(nearValue);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Far Clip Plane");
  ImGui::TableNextColumn();

  float farValue{obj.GetFarClipPlane()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("FarClip", &farValue);
  })) {
    obj.SetFarClipPlane(farValue);
  }


  auto viewport{obj.GetViewport()};
  auto constexpr viewport_drag_speed{0.01f};
  auto constexpr viewport_drag_min{0.0f};
  auto constexpr viewport_drag_max{1.0f};
  auto constexpr viewport_drag_format{"%.2f"};

  ImGui::TableNextColumn();
  ImGui::Text("Viewport Left");
  ImGui::TableNextColumn();

  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("##ViewportLeftDrag", &viewport.left, viewport_drag_speed, viewport_drag_min,
      viewport_drag_max,
      viewport_drag_format);
  })) {
    obj.SetViewport(viewport);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Viewport Top");
  ImGui::TableNextColumn();

  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("##ViewportTopDrag", &viewport.top, viewport_drag_speed, viewport_drag_min,
      viewport_drag_max,
      viewport_drag_format);
  })) {
    obj.SetViewport(viewport);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Viewport Right");
  ImGui::TableNextColumn();

  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("##ViewportRightDrag", &viewport.right, viewport_drag_speed, viewport_drag_min,
      viewport_drag_max, viewport_drag_format);
  })) {
    obj.SetViewport(viewport);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Viewport Bottom");
  ImGui::TableNextColumn();

  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("##ViewportBottomDrag", &viewport.bottom, viewport_drag_speed, viewport_drag_min,
      viewport_drag_max, viewport_drag_format);
  })) {
    obj.SetViewport(viewport);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Background Color");
  ImGui::TableNextColumn();

  Vector4 color{obj.GetBackgroundColor()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::ColorEdit4("###backgroundColor", color.GetData());
  })) {
    obj.SetBackgroundColor(color);
  }
}
}
