#include "oscillate_component_editor_drawer.hpp"

#include "editor_drawer_registry.hpp"
#include "gui_helpers.hpp"


namespace sorcery::mage {
auto OscillateComponentEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  OscillateComponent& obj,
  bool const allow_edit,
  bool& changed) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::Text("Direction");
  ImGui::TableNextColumn();

  auto direction{obj.GetDirection()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat3("###OscillateDirection", &direction[0], 0.01f, -100.0f, 100.0f, "%.2f");
  })) {
    obj.SetDirection(direction);
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Distance");
  ImGui::TableNextColumn();

  auto dist{obj.GetDistance()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("###OscillateDistance", &dist, 0.1f, 0.0f, 100.0f, "%.2f");
  })) {
    obj.SetDistance(dist);
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Speed");
  ImGui::TableNextColumn();

  auto speed{obj.GetSpeed()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("###OscillateSpeed", &speed, 0.1f, 0.0f, 100.0f, "%.2f");
  })) {
    obj.SetSpeed(speed);
    changed = true;
  }
}
}
