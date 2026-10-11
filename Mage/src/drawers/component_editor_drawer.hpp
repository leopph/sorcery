#pragma once

#include "editor_drawer.h"
#include "sorcery/scene/Component.hpp"


namespace sorcery::mage {
class ComponentEditorDrawer : public EditorDrawer<Component> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, Component& obj, bool allow_edit, bool& changed) -> void override;
};
}
