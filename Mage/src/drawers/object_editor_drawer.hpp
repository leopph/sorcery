#pragma once

#include "editor_drawer.h"
#include "Object.hpp"


namespace sorcery::mage {
class ObjectEditorDrawer : public EditorDrawer<Object> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, Object& obj, bool allow_edit, bool& changed) -> void override;
};
}
