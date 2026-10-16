#include "native_asset_importer.hpp"

#include "sorcery/host/assets/asset_manager.hpp"
#include "sorcery/assets/Material.hpp"
#include "sorcery/assets/Scene.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::mage::NativeAssetImporter>{"Native Asset Importer"}.
    REFLECT_REGISTER_ASSET_IMPORTER_CTOR;
}


namespace sorcery::mage {
auto NativeAssetImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  out.emplace_back(AssetManager::MATERIAL_ASSET_EXT);
  out.emplace_back(AssetManager::SCENE_ASSET_EXT);
}


auto NativeAssetImporter::Import(std::filesystem::path const& src,
                                    std::vector<AssetImportResult>& results) -> bool {
  auto const ext{src.extension()};

  if (ext != AssetManager::MATERIAL_ASSET_EXT && ext != AssetManager::SCENE_ASSET_EXT) {
    return false;
  }

  auto const imported_type{
    ext == AssetManager::MATERIAL_ASSET_EXT
      ? rttr::type::get<Material>()
      : ext == AssetManager::SCENE_ASSET_EXT
          ? rttr::type::get<Scene>()
          : rttr::type::get_by_name("")
  };

  results.emplace_back(AssetPackagePayloadKind::kInvalid, imported_type, src.filename().string(),
    std::vector<std::byte>{});
  return true;
}


auto NativeAssetImporter::IsNativeImporter() const noexcept -> bool {
  return true;
}
}
