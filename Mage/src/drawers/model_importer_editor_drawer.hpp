#pragma once

#include "editor_drawer.h"
#include "../AssetImporters/model_importer.hpp"


namespace sorcery::mage {
class ModelImporterEditorDrawer : public EditorDrawer<ModelImporter> {
  RTTR_ENABLE(EditorDrawer)
  auto Draw(EditorDrawerContext const& ctx, ModelImporter& obj, bool allow_edit, bool& changed) -> void override;
};
}
