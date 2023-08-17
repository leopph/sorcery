#include "NativeResourceImporter.hpp"

#include "../Resources/Material.hpp"
#include "../Resources/Scene.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::NativeResourceImporter>{"Native Resource Importer"}
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR;
}


namespace sorcery {
auto NativeResourceImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  out.emplace_back(ResourceManager::MATERIAL_RESOURCE_EXT);
  out.emplace_back(ResourceManager::SCENE_RESOURCE_EXT);
}


auto NativeResourceImporter::Import(std::filesystem::path const& src, std::vector<std::byte>& bytes, ExternalResourceCategory& categ) -> bool {
  auto const ext{src.extension()};
  return ext == ResourceManager::MATERIAL_RESOURCE_EXT || ext == ResourceManager::SCENE_RESOURCE_EXT;
}


auto NativeResourceImporter::GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type {
  if (resPathAbs.extension() == ResourceManager::MATERIAL_RESOURCE_EXT) {
    return rttr::type::get<Material>();
  }

  if (resPathAbs.extension() == ResourceManager::SCENE_RESOURCE_EXT) {
    return rttr::type::get<Scene>();
  }

  return rttr::type::get_by_name("");
}


bool NativeResourceImporter::IsNativeImporter() const noexcept {
  return true;
}
}
