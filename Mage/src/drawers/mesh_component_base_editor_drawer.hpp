#pragma once

#include "editor_drawer.h"
#include "MeshComponentBase.hpp"


namespace sorcery::mage {
class MeshComponentBaseEditorDrawer : public EditorDrawer<MeshComponentBase> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, MeshComponentBase& obj, bool allow_edit, bool& changed) -> void override;
};
}
