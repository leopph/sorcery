#include "asset_importer.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::mage::AssetImporter>{"Asset Importer"}
    .method("GetSupportedFileExtensions", &sorcery::mage::AssetImporter::GetSupportedFileExtensions)
    .method("Import", &sorcery::mage::AssetImporter::Import);
}


auto sorcery::mage::AssetImporter::IsNativeImporter() const noexcept -> bool {
  return false;
}
