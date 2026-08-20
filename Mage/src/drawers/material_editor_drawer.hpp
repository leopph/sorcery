#pragma once

#include "editor_drawer.h"
#include "resources/Material.hpp"


namespace sorcery::mage {
class MaterialEditorDrawer : public EditorDrawer<Material> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, Material& mtl, bool allow_edit, bool& changed) -> void override;
};
}
