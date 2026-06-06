#include "resource_importer.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::mage::ResourceImporter>{"Resource Importer"}
    .method("GetSupportedFileExtensions", &sorcery::mage::ResourceImporter::GetSupportedFileExtensions)
    .method("Import", &sorcery::mage::ResourceImporter::Import);
}


auto sorcery::mage::ResourceImporter::IsNativeImporter() const noexcept -> bool {
  return false;
}
