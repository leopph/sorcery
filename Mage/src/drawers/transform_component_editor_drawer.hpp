#pragma once

#include "editor_drawer.h"
#include "TransformComponent.hpp"


namespace sorcery::mage {
class TransformComponentEditorDrawer : public EditorDrawer<TransformComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, TransformComponent& obj, bool allow_edit, bool& changed) -> void override;
};
}
