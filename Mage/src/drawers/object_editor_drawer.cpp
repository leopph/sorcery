#include "object_editor_drawer.hpp"

#include "gui_helpers.hpp"


namespace sorcery::mage {
auto ObjectEditorDrawer::Draw(
  [[maybe_unused]] EditorDrawerContext const& ctx,
  Object& obj,
  [[maybe_unused]] bool const allow_edit,
  [[maybe_unused]] bool& changed
) -> void {
  ImGui::SeparatorText(std::format("{} ({})", obj.GetName(), rttr::type::get(obj).get_name().data()).c_str());
}
}
