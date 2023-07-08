#include <imgui.h>

#include "ObjectWrappers.hpp"


namespace sorcery::mage {
auto ObjectWrapperFor<CameraComponent>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto& cam{ dynamic_cast<CameraComponent&>(object) };

  auto const guidStr{ cam.GetGuid().ToString() };
  if (ImGui::BeginTable(std::format("{}", guidStr).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Type");
    ImGui::TableNextColumn();

    char const* const typeOptions[]{ "Perspective", "Orthographic" };
    int selection{ cam.GetType() == Camera::Type::Perspective ? 0 : 1 };
    if (ImGui::Combo("###CameraType", &selection, typeOptions, 2)) {
      cam.SetType(selection == 0 ? Camera::Type::Perspective : Camera::Type::Orthographic);
    }

    ImGui::TableNextColumn();

    if (cam.GetType() == Camera::Type::Perspective) {
      ImGui::Text("Field Of View");
      ImGui::TableNextColumn();
      float value{ cam.GetHorizontalPerspectiveFov() };
      if (ImGui::DragFloat(std::format("{}{}", guidStr, "FOV").c_str(), &value)) {
        cam.SetHorizontalPerspectiveFov(value);
      }
    } else {
      ImGui::Text("Size");
      ImGui::TableNextColumn();
      float value{ cam.GetHorizontalOrthographicSize() };
      if (ImGui::DragFloat(std::format("{}{}", guidStr, "OrthoSize").c_str(), &value)) {
        cam.SetHorizontalOrthographicSize(value);
      }
    }

    ImGui::TableNextColumn();
    ImGui::Text("Near Clip Plane");
    ImGui::TableNextColumn();

    float nearValue{ cam.GetNearClipPlane() };
    if (ImGui::DragFloat(std::format("{}{}", guidStr, "NearClip").c_str(), &nearValue)) {
      cam.SetNearClipPlane(nearValue);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Far Clip Plane");
    ImGui::TableNextColumn();

    float farValue{ cam.GetFarClipPlane() };
    if (ImGui::DragFloat(std::format("{}{}", guidStr, "FarClip").c_str(), &farValue)) {
      cam.SetFarClipPlane(farValue);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Viewport");
    ImGui::TableNextColumn();

    auto viewport{ cam.GetViewport() };
    if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportX").c_str(), &viewport.position.x)) {
      cam.SetViewport(viewport);
    }
    if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportY").c_str(), &viewport.position.y)) {
      cam.SetViewport(viewport);
    }
    if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportW").c_str(), &viewport.extent.width)) {
      cam.SetViewport(viewport);
    }
    if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportH").c_str(), &viewport.extent.height)) {
      cam.SetViewport(viewport);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Background Color");
    ImGui::TableNextColumn();

    Vector4 color{ cam.GetBackgroundColor() };
    if (ImGui::ColorEdit4("###backgroundColor", color.GetData())) {
      cam.SetBackgroundColor(color);
    }

    ImGui::EndTable();
  }
}
}
