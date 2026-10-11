#pragma once

#include "editor_drawer.h"
#include "sorcery/scene/CameraComponent.hpp"


namespace sorcery::mage {
class CameraComponentEditorDrawer : public EditorDrawer<CameraComponent> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, CameraComponent& obj, bool allow_edit, bool& changed) -> void override;
};
}
