#include "NativeResourceImporter.hpp"

#include "../Resources/Material.hpp"
#include "../Resources/Scene.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::NativeResourceImporter>{ "Native Resource Importer" }
    .constructor();
}


namespace sorcery {
char const* const NativeResourceImporter::MATERIAL_FILE_EXT{ ".mtl" };
char const* const NativeResourceImporter::SCENE_FILE_EXT{ ".scene" };


auto NativeResourceImporter::GetSupportedFileExtensions(std::vector<std::string>& out) -> void {
  out.emplace_back(MATERIAL_FILE_EXT);
  out.emplace_back(SCENE_FILE_EXT);
}


auto NativeResourceImporter::Import(std::filesystem::path const& src) -> std::shared_ptr<Resource> {
  if (!src.has_extension()) {
    return nullptr;
  }

  auto const yamlNode{ YAML::LoadFile(src.string()) };

  if (!yamlNode.IsDefined() || yamlNode.IsNull()) {
    return nullptr;
  }

  std::shared_ptr<NativeResource> nativeRes;

  if (src.extension() == MATERIAL_FILE_EXT) {
    nativeRes = std::make_shared<Material>();
  } else if (src.extension() == SCENE_FILE_EXT) {
    nativeRes = std::make_shared<Scene>();
  }

  if (nativeRes) {
    nativeRes->Deserialize(yamlNode);
  }

  return nativeRes;
}


auto NativeResourceImporter::GetPrecedence() const noexcept -> int {
  return 0;
}
}
