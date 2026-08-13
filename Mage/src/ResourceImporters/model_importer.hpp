#pragma once

#include <cstdint>

#include "material_import.hpp"
#include "mesh_import.hpp"
#include "resource_importer.hpp"
#include "texture_import.hpp"


namespace sorcery::mage {
class ModelImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)
  RTTR_REGISTRATION_FRIEND

public:
  auto GetSupportedFileExtensions(
    std::pmr::vector<std::string>& out
  ) -> void override;

  [[nodiscard]]
  auto Import(
    std::filesystem::path const& src,
    std::vector<ResourceImportResult>& results
  ) -> bool override;

private:
  enum class SubresourceKind : std::uint32_t {
    kMaterial = 1,
    kTexture  = 2
  };


  struct SubresourceImportSettings {
    MaterialImportSettings material_settings;
    TextureImportSettings texture_settings;
    SubresourceKind kind{SubresourceKind::kMaterial};
  };


  std::vector<SubresourceImportSettings> subresource_import_settings_;
  MeshImportSettings mesh_import_settings_;
  bool import_materials_{true};
  bool import_textures_{true};
};
}
