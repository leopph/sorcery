#pragma once

#include "editor_drawer.h"
#include "SkinnedMeshComponent.hpp"


namespace sorcery::mage {
class SkinnedMeshComponentEditorDrawer : public EditorDrawer<SkinnedMeshComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, SkinnedMeshComponent& obj, bool allow_edit, bool& changed) -> void override;
};
}
