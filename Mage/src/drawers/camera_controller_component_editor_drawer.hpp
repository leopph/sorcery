#pragma once

#include "editor_drawer.h"
#include "sorcery/scene/CameraControllerComponent.hpp"


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
