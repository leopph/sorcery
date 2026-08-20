#include "transform_component_editor_drawer.hpp"

#include "editor_drawer_registry.hpp"


namespace sorcery::mage {
auto TransformComponentEditorDrawer::Draw
(EditorDrawerContext const& ctx,
 TransformComponent& obj,
 bool const allow_edit,
 bool& changed
) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::BeginDisabled(!allow_edit);

  ImGui::Text("Local Position");
  ImGui::TableNextColumn();

  Vector3 localPos{obj.GetLocalPosition()};
  if (ImGui::DragFloat3("###transformPos", localPos.GetData(), 0.1f)) {
    obj.SetLocalPosition(localPos);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Local Rotation");
  ImGui::TableNextColumn();

  if (auto euler{obj.GetLocalEulerAngles()}; ImGui::DragFloat3("###transformRot", euler.GetData(), 1.0f)) {
    obj.SetLocalEulerAngles(euler);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Local Scale");
  ImGui::TableNextColumn();

  auto static uniformScale{true};
  auto constexpr scaleSpeed{0.01f};

  ImGui::Text("%s", "Uniform");
  ImGui::SameLine();
  ImGui::Checkbox("##UniformScaleCheck", &uniformScale);
  ImGui::SameLine();

  if (uniformScale) {
    f32 scale{obj.GetLocalScale()[0]};
    if (ImGui::DragFloat("###transformScale", &scale, scaleSpeed)) {
      obj.SetLocalScale(Vector3{scale});
    }
  } else {
    Vector3 localScale{obj.GetLocalScale()};
    if (ImGui::DragFloat3("###transformScale", localScale.GetData(), scaleSpeed)) {
      obj.SetLocalScale(localScale);
    }
  }

  ImGui::EndDisabled();
}
}
