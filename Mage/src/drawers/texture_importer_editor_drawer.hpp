#pragma once

#include "editor_drawer.h"
#include "../ResourceImporters/texture_importer.hpp"


namespace sorcery::mage {
class TextureImporterEditorDrawer : public EditorDrawer<TextureImporter> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, TextureImporter& obj, bool allow_edit, bool& changed) -> void override;
};
}
