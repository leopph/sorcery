#pragma once

#include "material_import.hpp"
#include "texture_import.hpp"


namespace sorcery::mage {
// Returns whether the function changed any properties.
[[nodiscard]]
auto DrawMaterialImportSettings(MaterialImportSettings& settings, bool allow_edit, bool& changed) -> bool;
// Returns whether the function changed any properties.
[[nodiscard]]
auto DrawTextureImportSettings(TextureImportSettings& settings, bool allow_edit, bool& changed) -> bool;
}
