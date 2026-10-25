#include "environment_component_editor_drawer.hpp"

#include <imgui.h>

#include "editor_drawer_registry.hpp"
#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto EnvironmentComponentEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  EnvironmentComponent& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::TextUnformatted("Ambient Light");
  ImGui::TableNextColumn();

  if (auto color = obj.GetAmbientLightVector(); ImGuiDisabled(!allow_edit, [&] {
    return ImGui::ColorEdit3("Ambient Light", color.GetData());
  })) {
    obj.SetAmbientLightVector(color);
  }

  ImGui::TableNextColumn();
  ImGui::TextUnformatted("Sky Mode");
  ImGui::TableNextColumn();

  constexpr char const* sky_modes[]{"Color", "Skybox"};
  auto sky_mode_selection = obj.GetSkyMode() == SkyMode::kColor ? 0 : 1;

  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::Combo("##SkyModeCombo", &sky_mode_selection, sky_modes, 2);
  })) {
    obj.SetSkyMode(sky_mode_selection == 0 ? SkyMode::kColor : SkyMode::kSkybox);
  }

  ImGui::TableNextColumn();

  if (obj.GetSkyMode() == SkyMode::kColor) {
    ImGui::TextUnformatted("Color");
    ImGui::TableNextColumn();
    if (auto sky_color = obj.GetSkyColor(); ImGuiDisabled(!allow_edit, [&] {
      return ImGui::ColorEdit3("##skyColor", sky_color.GetData());
    })) {
      obj.SetSkyColor(sky_color);
    }
  } else if (obj.GetSkyMode() == SkyMode::kSkybox) {
    ImGui::TextUnformatted("Skybox");
    ImGui::TableNextColumn();
    static ObjectPicker<Cubemap> skybox_picker;
    if (auto skybox = obj.GetSkybox().Get(); ImGuiDisabled(!allow_edit, [&] {
      return skybox_picker.Draw(skybox);
    })) {
      obj.SetSkybox(MakeAssetRef(skybox.Get()));
    }
  } else {
    ImGui::TextColored(ImVec4{1, 1, 0, 1}, "%s", "Unknown sky mode.");
  }
}
}
