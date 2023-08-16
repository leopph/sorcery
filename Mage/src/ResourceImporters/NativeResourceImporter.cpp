#include "NativeResourceImporter.hpp"

#include "../Resources/Material.hpp"
#include "../Resources/Scene.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::NativeResourceImporter>{"Native Resource Importer"}
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR;
}


namespace sorcery {
auto NativeResourceImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  out.emplace_back(MATERIAL_FILE_EXT);
  out.emplace_back(SCENE_FILE_EXT);
}


auto NativeResourceImporter::Import(std::filesystem::path const& src, std::vector<std::byte>& bytes) -> bool {
  if (!src.has_extension()) {
    return false; // TODO
  }

  auto const yamlNode{YAML::LoadFile(src.string())};

  if (!yamlNode.IsDefined()) {
    return false; // TODO
  }

  ObserverPtr<NativeResource> nativeRes{nullptr};

  if (src.extension() == MATERIAL_FILE_EXT) {
    nativeRes = new Material{};
  } else if (src.extension() == SCENE_FILE_EXT) {
    nativeRes = new Scene{};
  }

  if (nativeRes) {
    nativeRes->Deserialize(yamlNode);
  }

  return true; // TODO
}


auto NativeResourceImporter::GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type {
  if (resPathAbs.extension() == MATERIAL_FILE_EXT) {
    return rttr::type::get<Material>();
  }

  if (resPathAbs.extension() == SCENE_FILE_EXT) {
    return rttr::type::get<Scene>();
  }

  return rttr::type::get_by_name("");
}
}
