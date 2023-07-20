#include "ResourceImporter.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::ResourceImporter>{ "Resource Importer" }
    .method("GetSupportedFileExtensions", &sorcery::ResourceImporter::GetSupportedFileExtensions)
    .method("Import", &sorcery::ResourceImporter::Import)
    .method("GetPrecedence", &sorcery::ResourceImporter::GetPrecedence);
}
