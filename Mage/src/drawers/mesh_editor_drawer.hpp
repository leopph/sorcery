#pragma once

#include "editor_drawer.h"
#include "sorcery/assets/Mesh.hpp"


namespace sorcery::mage {
class MeshEditorDrawer : public EditorDrawer<Mesh> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, Mesh& mesh, bool allow_edit, bool& changed) -> void override;
};
}
