#include "native_resource_importer.hpp"

#include "resource_manager.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/Scene.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::mage::NativeResourceImporter>{"Native Resource Importer"}.
    REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR;
}


namespace sorcery::mage {
auto NativeResourceImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  out.emplace_back(ResourceManager::MATERIAL_RESOURCE_EXT);
  out.emplace_back(ResourceManager::SCENE_RESOURCE_EXT);
}


auto NativeResourceImporter::Import(std::filesystem::path const& src,
                                    std::vector<ResourceImportResult>& results) -> bool {
  auto const ext{src.extension()};

  if (ext != ResourceManager::MATERIAL_RESOURCE_EXT && ext != ResourceManager::SCENE_RESOURCE_EXT) {
    return false;
  }

  auto const imported_type{
    ext == ResourceManager::MATERIAL_RESOURCE_EXT
      ? rttr::type::get<Material>()
      : ext == ResourceManager::SCENE_RESOURCE_EXT
          ? rttr::type::get<Scene>()
          : rttr::type::get_by_name("")
  };

  results.emplace_back(ResourceDesc{imported_type, std::nullopt});
  return true;
}


auto NativeResourceImporter::IsNativeImporter() const noexcept -> bool {
  return true;
}
}
