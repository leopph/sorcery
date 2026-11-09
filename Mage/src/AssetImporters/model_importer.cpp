#include "model_importer.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <assimp/Importer.hpp>
#include <spdlog/spdlog.h>

#include "model_import/identity.hpp"
#include "model_import/materials.hpp"
#include "model_import/meshes.hpp"
#include "model_import/prefab.hpp"
#include "model_import/scene.hpp"
#include "model_import/types.hpp"
#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/assets/Mesh.hpp"
#include "sorcery/host/app.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::mage::ModelImporter::MaterialSettingsEntry>("ModelImporterMaterialSettingsEntry")
    .constructor<>()(rttr::policy::ctor::as_object)
    .property("Key", &sorcery::mage::ModelImporter::MaterialSettingsEntry::key)
    .property("Settings", &sorcery::mage::ModelImporter::MaterialSettingsEntry::settings);

  rttr::registration::class_<sorcery::mage::ModelImporter::TextureSettingsEntry>("ModelImporterTextureSettingsEntry")
    .constructor<>()(rttr::policy::ctor::as_object)
    .property("Key", &sorcery::mage::ModelImporter::TextureSettingsEntry::key)
    .property("Settings", &sorcery::mage::ModelImporter::TextureSettingsEntry::settings);

  rttr::registration::class_<sorcery::mage::ModelImporter>{"Model Importer"}
    .REFLECT_REGISTER_ASSET_IMPORTER_CTOR
    .property("Materials", &sorcery::mage::ModelImporter::material_settings_)
    .property("Textures", &sorcery::mage::ModelImporter::texture_settings_)
    .property("Mesh Import Settings", &sorcery::mage::ModelImporter::mesh_import_settings_)
    .property("Import materials", &sorcery::mage::ModelImporter::import_materials_)
    .property("Import textures", &sorcery::mage::ModelImporter::import_textures_);
}


namespace sorcery::mage {
namespace {
template<class Entry>
auto FindSettings(
  std::span<Entry const> const entries,
  std::string_view const key
) -> std::optional<decltype(Entry::settings)> {
  auto const it = std::ranges::lower_bound(entries, key, {}, &Entry::key);

  if (it != entries.end() && it->key == key) {
    return std::make_optional(it->settings);
  }

  return std::nullopt;
}
}


auto ModelImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  std::string extensions;
  Assimp::Importer const importer;
  importer.GetExtensionList(extensions);

  // Assimp extension list format is "*.3ds;*.obj;*.dae"
  for (auto const ext : std::views::split(extensions, std::string_view{";"})) {
    out.emplace_back(std::string_view{std::begin(ext), std::end(ext)}.substr(1));
  }
}


auto ModelImporter::Import(std::filesystem::path const& src, std::vector<AssetImportResult>& results) -> bool {
  auto const importer = model_import::LoadScene(src);

  if (!importer) {
    return false;
  }

  auto const& scene{*importer->GetScene()};

  // Import meshes

  auto mesh_import = model_import::ImportMeshes(scene, src, mesh_import_settings_, App::Instance().GetJobSystem());

  if (!mesh_import) {
    spdlog::error("Failed to import meshes from \"{}\".", ToUntypedSv(src.u8string()));
    return false;
  }

  // Import materials and gather info to load textures

  auto [mtl_slots, mtl_assets, texes_to_load] = model_import::CollectMaterials(scene, import_materials_,
    import_textures_);
  auto const tex_coverage_thresholds = model_import::ComputeTextureCoverageThresholds(mtl_assets, texes_to_load.size());

  //  At this point, the submeshes inside each imported mesh have absolute material indices, not local to their parent mesh.
  // We can fix those up now and generate a table that describes the remap. Useful later.
  auto const mtl_slots_by_mesh = model_import::AssignMeshMaterialSlots(mesh_import->meshes, mtl_slots);

  // Nothing depends on the actual texture assets so we can load them last

  if (import_textures_) {
    AppendUnreferencedTextures(scene, texes_to_load);
  }

  // Stable subasset identities.
  model_import::SubassetKeys const subasset_keys{
    .prefab = "prefab",
    .meshes = std::move(mesh_import->keys),
    .materials = import_materials_
                   ? model_import::MakeMaterialKeys(mtl_slots)
                   : std::vector<std::string>{},
    .textures = MakeTextureKeys(texes_to_load),
  };

  spdlog::debug("Subasset keys of \"{}\":", ToUntypedSv(src.u8string()));
  spdlog::debug("  {}", subasset_keys.prefab);

  for (auto const& key : std::views::join(std::array{
         std::span{subasset_keys.meshes}, std::span{subasset_keys.materials}, std::span{subasset_keys.textures}
       })) {
    spdlog::debug("  {}", key);
  }

  // Sync settings with current import keys

  if (import_materials_) {
    std::vector<MaterialSettingsEntry> new_settings_entries;
    new_settings_entries.reserve(subasset_keys.materials.size());

    std::ranges::sort(material_settings_, {}, &MaterialSettingsEntry::key);

    for (auto const& key : subasset_keys.materials) {
      auto const settings = FindSettings<MaterialSettingsEntry>(material_settings_, key).value_or(
        MaterialImportSettings{});
      new_settings_entries.emplace_back(key, settings);
    }

    std::ranges::sort(new_settings_entries, {}, &MaterialSettingsEntry::key);
    material_settings_ = std::move(new_settings_entries);
  }

  std::vector<TextureImportSettings> tex_settings;

  if (import_textures_) {
    tex_settings.reserve(subasset_keys.textures.size());

    std::vector<TextureSettingsEntry> new_settings_entries;
    new_settings_entries.reserve(subasset_keys.textures.size());

    std::ranges::sort(texture_settings_, {}, &TextureSettingsEntry::key);

    for (auto const [i, key] : std::views::enumerate(subasset_keys.textures)) {
      auto const settings = FindSettings<TextureSettingsEntry>(texture_settings_, key).value_or(
        TextureImportSettings{
          .type = TextureImportType::kTexture2D,
          .allow_block_compression = true,
          .is_srgb = model_import::IsSrgb(texes_to_load[i].type),
          .generate_mips = true
        });
      new_settings_entries.emplace_back(key, settings);
      tex_settings.emplace_back(settings);
    }

    std::ranges::sort(new_settings_entries, {}, &TextureSettingsEntry::key);
    texture_settings_ = std::move(new_settings_entries);
  }

  auto const subasset_ids = model_import::SubassetIds::FromKeys(subasset_keys, src);

  // Now we have subasset ids so we can patch material->texture references
  ResolveTextureIds(mtl_assets, subasset_ids);

  // Write subassets to output in package order

  if (mesh_import->hierarchy) {
    results.emplace_back(CreateHierarchyPrefab(ToUntypedSv(src.filename().stem().u8string()), mesh_import->meshes,
      mtl_slots_by_mesh, *mesh_import->hierarchy, subasset_ids));
  } else {
    results.emplace_back(CreateSingleEntityPrefab(ToUntypedSv(src.filename().stem().u8string()),
      mesh_import->meshes.at(0), mtl_slots_by_mesh.at(0), subasset_ids));
  }

  for (auto const [mesh_idx, mesh_data] : std::views::enumerate(mesh_import->meshes)) {
    auto mesh_bytes = model_import::SerializeMesh(mesh_data);
    results.emplace_back(subasset_ids.GetMeshId(mesh_idx), GetCurrentPayloadVersion(AssetPackagePayloadKind::kMesh),
      AssetPackagePayloadKind::kMesh, rttr::type::get<Mesh>(), std::string{ToUntypedSv(src.stem().u8string())},
      std::move(mesh_bytes));
  }

  if (import_materials_) {
    std::ranges::move(BuildMaterialResults(mtl_assets, mtl_slots, subasset_ids), std::back_inserter(results));
  }

  if (import_textures_) {
    auto tex_results = BuildTextureResults(src, texes_to_load, tex_coverage_thresholds, tex_settings, subasset_ids);

    if (!tex_results) {
      return false;
    }

    std::ranges::move(*tex_results, std::back_inserter(results));
  }

  return true;
}


auto ModelImporter::GetMeshImportSettings() const -> MeshImportSettings const& {
  return mesh_import_settings_;
}


auto ModelImporter::SetMeshImportSettings(MeshImportSettings const& settings) -> void {
  mesh_import_settings_ = settings;
}


auto ModelImporter::GetMaterialImportSettings() const -> std::span<MaterialSettingsEntry const> {
  return material_settings_;
}


auto ModelImporter::SetMaterialImportSettings(std::span<MaterialSettingsEntry const> const settings) -> void {
  material_settings_.assign_range(settings);
  std::ranges::sort(material_settings_, {}, &MaterialSettingsEntry::key);
}


auto ModelImporter::GetTextureImportSettings() const -> std::span<TextureSettingsEntry const> {
  return texture_settings_;
}


auto ModelImporter::SetTextureImportSettings(std::span<TextureSettingsEntry const> const settings) -> void {
  texture_settings_.assign_range(settings);
  std::ranges::sort(texture_settings_, {}, &TextureSettingsEntry::key);
}


auto ModelImporter::IsMaterialImportEnabled() const -> bool {
  return import_materials_;
}


auto ModelImporter::SetMaterialImportEnabled(bool const enabled) -> void {
  import_materials_ = enabled;
}


auto ModelImporter::IsTextureImportEnabled() const -> bool {
  return import_textures_;
}


auto ModelImporter::SetTextureImportEnabled(bool const enabled) -> void {
  import_textures_ = enabled;
}
}
