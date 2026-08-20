#pragma once

#include "editor_drawer.h"
#include "Texture2D.hpp"


namespace sorcery::mage {
class Texture2DEditorDrawer : public EditorDrawer<Texture2D> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, Texture2D& obj, bool allow_edit, bool& changed) -> void override;
};
}
