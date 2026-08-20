#include "component_editor_drawer.hpp"


namespace sorcery::mage {
auto ComponentEditorDrawer::Draw(
  [[maybe_unused]] EditorDrawerContext const& ctx,
  [[maybe_unused]] Component& obj,
  [[maybe_unused]] bool const allow_edit,
  [[maybe_unused]] bool& changed
) -> void {
  // We explicitly do not call DrawAs<SceneObject> here to avoid displaying the name and type
}
}
