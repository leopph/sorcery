#pragma once

#include "editor_drawer.h"
#include "sorcery/scene/animator_component.hpp"


namespace sorcery::mage {
class AnimatorComponentEditorDrawer : public EditorDrawer<AnimatorComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, AnimatorComponent& obj, bool allow_edit, bool& changed) -> void override;
};
}
