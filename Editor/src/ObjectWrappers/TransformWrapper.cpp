#include <imgui.h>

#include "ObjectWrappers.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<TransformComponent>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto& transform{ dynamic_cast<TransformComponent&>(object) };

  if (ImGui::BeginTable(std::format("{}", transform.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Local Position");
    ImGui::TableNextColumn();

    Vector3 localPos{ transform.GetLocalPosition() };
    if (ImGui::DragFloat3("###transformPos", localPos.GetData(), 0.1f)) {
      transform.SetLocalPosition(localPos);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Local Rotation");
    ImGui::TableNextColumn();

    auto euler{ transform.GetLocalRotation().ToEulerAngles() };
    if (ImGui::DragFloat3("###transformRot", euler.GetData(), 1.0f)) {
      transform.SetLocalRotation(Quaternion::FromEulerAngles(euler));
    }

    ImGui::TableNextColumn();
    ImGui::Text("Local Scale");
    ImGui::TableNextColumn();

    bool static uniformScale{ true };
    auto constexpr scaleSpeed{ 0.01f };

    ImGui::Text("%s", "Uniform");
    ImGui::SameLine();
    ImGui::Checkbox("##UniformScaleCheck", &uniformScale);
    ImGui::SameLine();

    if (uniformScale) {
      f32 scale{ transform.GetLocalScale()[0] };
      if (ImGui::DragFloat("###transformScale", &scale, scaleSpeed)) {
        transform.SetLocalScale(Vector3{ scale });
      }
    } else {
      Vector3 localScale{ transform.GetLocalScale() };
      if (ImGui::DragFloat3("###transformScale", localScale.GetData(), scaleSpeed)) {
        transform.SetLocalScale(localScale);
      }
    }

    ImGui::EndTable();
  }
}
}
