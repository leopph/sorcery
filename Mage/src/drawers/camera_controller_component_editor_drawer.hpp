#pragma once

#include "CameraControllerComponent.hpp"
#include "editor_drawer.h"


namespace sorcery::mage {
class CameraControllerComponentEditorDrawer : public EditorDrawer<CameraControllerComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx,
            CameraControllerComponent& obj,
            bool allow_edit,
            bool& changed
  ) -> void override;
};
}
