#include "texture_importer_editor_drawer.hpp"

#include "import_settings_editor_draw_helpers.hpp"


namespace sorcery::mage {
auto TextureImporterEditorDrawer::Draw(
  [[maybe_unused]] EditorDrawerContext const& ctx,
  TextureImporter& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  if (auto settings{obj.GetSettings()}; DrawTextureImportSettings(settings, allow_edit, changed)) {
    obj.SetSettings(settings);
  }
}
}
