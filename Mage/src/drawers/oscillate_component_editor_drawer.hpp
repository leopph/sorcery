#pragma once

#include "editor_drawer.h"
#include "OscillateComponent.h"


namespace sorcery::mage {
class OscillateComponentEditorDrawer : public EditorDrawer<OscillateComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, OscillateComponent& obj, bool allow_edit, bool& changed) -> void override;
};
}
