#pragma once

#include "editor_drawer.h"
#include "Entity.hpp"


namespace sorcery::mage {
class EntityEditorDrawer : public EditorDrawer<Entity> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, Entity& obj, bool allow_edit, bool& changed) -> void override;
};
}
