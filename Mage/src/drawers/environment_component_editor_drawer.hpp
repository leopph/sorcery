#pragma once

#include "editor_drawer.h"
#include "sorcery/scene/environment_component.hpp"


namespace sorcery::mage {
class EnvironmentComponentEditorDrawer : public EditorDrawer<EnvironmentComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, EnvironmentComponent& obj, bool allow_edit, bool& changed) -> void override;
};
}
