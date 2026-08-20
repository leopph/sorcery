#pragma once

#include "Component.hpp"
#include "editor_drawer.h"


namespace sorcery::mage {
class ComponentEditorDrawer : public EditorDrawer<Component> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, Component& obj, bool allow_edit, bool& changed) -> void override;
};
}
