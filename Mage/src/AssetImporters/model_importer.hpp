#pragma once

#include "asset_importer.hpp"
#include "sorcery/host/assets/material_import.hpp"
#include "sorcery/host/assets/mesh_import.hpp"
#include "sorcery/host/assets/texture_import.hpp"


namespace sorcery::mage {
class ModelImporter final : public AssetImporter {
  RTTR_ENABLE(AssetImporter)
  RTTR_REGISTRATION_FRIEND

public:
  struct MaterialSettingsEntry {
    std::string key;
    std::string name;
    MaterialImportSettings settings;
  };


  struct TextureSettingsEntry {
    std::string key;
    std::string name;
    TextureImportSettings settings;
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
  auto GetMaterialImportSettings() const -> std::span<MaterialSettingsEntry const>;
  auto SetMaterialImportSettings(std::span<MaterialSettingsEntry const> settings) -> void;

  [[nodiscard]]
  auto GetTextureImportSettings() const -> std::span<TextureSettingsEntry const>;
  auto SetTextureImportSettings(std::span<TextureSettingsEntry const> settings) -> void;

  [[nodiscard]]
  auto IsMaterialImportEnabled() const -> bool;

  auto SetMaterialImportEnabled(bool enabled) -> void;

  [[nodiscard]]
  auto IsTextureImportEnabled() const -> bool;

  auto SetTextureImportEnabled(bool enabled) -> void;

private:
  std::vector<MaterialSettingsEntry> material_settings_; // sorted by key
  std::vector<TextureSettingsEntry> texture_settings_; // sorted by key

  MeshImportSettings mesh_import_settings_;
  bool import_materials_{true};
  bool import_textures_{true};
};
}
