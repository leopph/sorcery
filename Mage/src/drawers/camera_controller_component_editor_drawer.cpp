#include "camera_controller_component_editor_drawer.hpp"

#include <imgui.h>

#include "editor_drawer_registry.hpp"
#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto CameraControllerComponentEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  CameraControllerComponent& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::Text("Mouse Sensitivity");
  ImGui::TableNextColumn();

  auto mouse_sens{obj.get_mouse_sens()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("###CamCtrlMouseSens", &mouse_sens, 0.05f, 0.1f, 10.0f, "%.2f");
  })) {
    obj.set_mouse_sens(mouse_sens);
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Move Speed");
  ImGui::TableNextColumn();

  auto move_speed{obj.get_move_speed()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("###CamCtrlMoveSpeed", &move_speed, 0.1f, 0.1f, 100.0f, "%.2f");
  })) {
    obj.set_move_speed(move_speed);
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Sprint Multiplier");
  ImGui::TableNextColumn();

  auto sprint_multiplier{obj.get_sprint_multiplier()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("###CamCtrlSprintMul", &sprint_multiplier, 0.1f, 1.0f, 10.0f, "%.2f");
  })) {
    obj.set_sprint_multiplier(sprint_multiplier);
    changed = true;
  }
}
}
