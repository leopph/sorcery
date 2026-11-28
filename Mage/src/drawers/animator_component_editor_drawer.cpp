#include "animator_component_editor_drawer.hpp"

#include "editor_drawer_registry.hpp"


namespace sorcery::mage {
auto AnimatorComponentEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  AnimatorComponent& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::TextUnformatted("Animation clip");
  ImGui::TableNextColumn();

  static ObjectPicker<AnimationClip> clip_picker;
  if (auto clip = obj.GetClip().Get(); ImGuiDisabled(!allow_edit, [&] {
    return clip_picker.Draw(clip);
  })) {
    obj.SetClip(MakeAssetRef(clip.Get()));
  }
}
}
