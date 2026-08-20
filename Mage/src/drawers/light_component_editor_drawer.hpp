#pragma once

#include "editor_drawer.h"
#include "LightComponents.hpp"


namespace sorcery::mage {
class LightComponentEditorDrawer : public EditorDrawer<LightComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, LightComponent& obj, bool allow_edit, bool& changed) -> void override;
};
}
