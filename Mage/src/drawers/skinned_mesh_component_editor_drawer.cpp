#include "skinned_mesh_component_editor_drawer.hpp"

#include "editor_drawer_registry.hpp"
#include "gui_helpers.hpp"


namespace sorcery::mage {
auto SkinnedMeshComponentEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  SkinnedMeshComponent& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<MeshComponentBase>(obj, allow_edit, changed);

  ImGui::TableNextColumn();
  ImGui::Text("Animation");

  ImGui::TableNextColumn();
  std::vector<char const*> items;
  items.emplace_back("None");

  if (auto const mesh{obj.GetMesh()}) {
    for (auto const& [name, duration, ticks_per_second, node_anims] : mesh->GetAnimations()) {
      items.emplace_back(name.c_str());
    }
  }

  auto const cur_animation_idx{obj.GetCurrentAnimationIndex()};
  if (auto combo_idx{static_cast<int>(cur_animation_idx ? *cur_animation_idx + 1 : 0)};
    ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Combo("##animCombo", &combo_idx, items.data(), static_cast<int>(items.size()));
    })) {
    obj.SetCurrentAnimationIndex(combo_idx == 0 ? std::nullopt : std::make_optional(combo_idx - 1));
  }
}
}
