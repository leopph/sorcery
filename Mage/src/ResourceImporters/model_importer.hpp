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
  enum class SubresourceKind : std::uint32_t {
    kMaterial = 1,
    kTexture  = 2
  };


  struct SubresourceImportSettings {
    MaterialImportSettings material_settings;
    TextureImportSettings texture_settings;
    SubresourceKind kind{SubresourceKind::kMaterial};
  };


  auto GetSupportedFileExtensions(
    std::pmr::vector<std::string>& out
  ) -> void override;

  [[nodiscard]]
  auto Import(
    std::filesystem::path const& src,
    std::vector<ResourceImportResult>& results
  ) -> bool override;

  [[nodiscard]]
  auto GetMeshImportSettings() const -> MeshImportSettings const&;

  auto SetMeshImportSettings(MeshImportSettings const& settings) -> void;

  [[nodiscard]]
  auto GetSubresourceImportSettings() const -> std::span<SubresourceImportSettings const>;

  auto SetSubresourceImportSettings(std::span<SubresourceImportSettings const> settings) -> void;

  [[nodiscard]]
  auto IsMaterialImportEnabled() const -> bool;

  auto SetMaterialImportEnabled(bool enabled) -> void;

  [[nodiscard]]
  auto IsTextureImportEnabled() const -> bool;

  auto SetTextureImportEnabled(bool enabled) -> void;

private:
  std::vector<SubresourceImportSettings> subresource_import_settings_;
  MeshImportSettings mesh_import_settings_;
  bool import_materials_{true};
  bool import_textures_{true};
};
}
