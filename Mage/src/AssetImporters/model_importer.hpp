#pragma once

#include <cstdint>

#include "asset_importer.hpp"
#include "sorcery/host/assets/material_import.hpp"
#include "sorcery/host/assets/mesh_import.hpp"
#include "sorcery/host/assets/texture_import.hpp"


namespace sorcery::mage {
class ModelImporter final : public AssetImporter {
  RTTR_ENABLE(AssetImporter)
  RTTR_REGISTRATION_FRIEND

public:
  enum class SubassetKind : std::uint32_t {
    kMaterial = 1,
    kTexture  = 2
  };


  struct SubassetImportSettings {
    MaterialImportSettings material_settings;
    TextureImportSettings texture_settings;
    SubassetKind kind{SubassetKind::kMaterial};
  };


  auto GetSupportedFileExtensions(
    std::pmr::vector<std::string>& out
  ) -> void override;

  [[nodiscard]]
  auto Import(
    std::filesystem::path const& src,
    std::vector<AssetImportResult>& results
  ) -> bool override;

  [[nodiscard]]
  auto GetMeshImportSettings() const -> MeshImportSettings const&;

  auto SetMeshImportSettings(MeshImportSettings const& settings) -> void;

  [[nodiscard]]
  auto GetSubassetImportSettings() const -> std::span<SubassetImportSettings const>;

  auto SetSubassetImportSettings(std::span<SubassetImportSettings const> settings) -> void;

  [[nodiscard]]
  auto IsMaterialImportEnabled() const -> bool;

  auto SetMaterialImportEnabled(bool enabled) -> void;

  [[nodiscard]]
  auto IsTextureImportEnabled() const -> bool;

  auto SetTextureImportEnabled(bool enabled) -> void;

private:
  std::vector<SubassetImportSettings> subasset_import_settings_;
  MeshImportSettings mesh_import_settings_;
  bool import_materials_{true};
  bool import_textures_{true};
};
}
