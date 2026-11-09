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
#include "model_import/skinning.hpp"
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

  auto const has_bones = std::ranges::any_of(std::span{scene.mMeshes, scene.mNumMeshes},
    [](auto const* const mesh) { return mesh->HasBones(); });

  auto const bake_into_single_mesh = has_bones || mesh_import_settings_.import_as_single_mesh;

  std::vector<MeshData> imported_meshes;
  std::vector<model_import::SceneNode> nodes;
  model_import::NodesByMesh nodes_by_mesh;
  model_import::MeshesByGroup meshes_by_group;
  model_import::GroupsByNode groups_by_node;

  if (bake_into_single_mesh) {
    auto& mesh_data = imported_meshes.emplace_back();

    std::vector<model_import::SourceMesh> meshes;
    std::vector<model_import::SourceBone> bone_proc_info;
    CollectBakedMeshes(scene, src, meshes, bone_proc_info);

    auto const skeleton_node_name_to_idx = BuildSkeleton(*scene.mRootNode, bone_proc_info, mesh_data.skeleton,
      mesh_data.bones);

    meshes = FuseByMaterial(meshes);

    if (!BuildMeshGeometry(meshes, mesh_import_settings_.force_idx32, App::Instance().GetJobSystem(), mesh_data)) {
      return false;
    }

    mesh_data.animations = model_import::CollectAnimations(scene, skeleton_node_name_to_idx);
  } else {
    auto const meshes = model_import::CollectSourceMeshes(scene, src);
    nodes = model_import::FlattenNodeHierarchy(scene);
    nodes_by_mesh = CollectNodesByMesh(nodes, meshes.size());
    meshes_by_group = CollectMeshesByGroup(nodes_by_mesh, meshes);
    groups_by_node = model_import::CollectGroupsByNode(meshes_by_group, nodes_by_mesh, nodes.size());

    for (auto const [group_idx, mesh_indices] : std::views::enumerate(meshes_by_group)) {
      auto const group_meshes = mesh_indices | std::views::transform([&meshes](auto const mesh_idx) -> auto const& {
        return meshes[mesh_idx].value();
      });

      MeshData mesh_data;

      if (!model_import::BuildMeshGeometry(group_meshes, mesh_import_settings_.force_idx32,
        App::Instance().GetJobSystem(),
        mesh_data)) {
        return false;
      }

      imported_meshes.emplace_back(std::move(mesh_data));
    }
  }

  // Import materials and gather info to load textures

  auto [mtl_slots, mtl_assets, texes_to_load] = model_import::CollectMaterials(scene, import_materials_,
    import_textures_);
  auto const tex_coverage_thresholds = model_import::ComputeTextureCoverageThresholds(mtl_assets, texes_to_load.size());

  //  At this point, the submeshes inside each imported mesh have absolute material indices, not local to their parent mesh.
  // We can fix those up now and generate a table that describes the remap. Useful later.
  auto const mtl_slots_by_mesh = model_import::AssignMeshMaterialSlots(imported_meshes, mtl_slots);

  // Nothing depends on the actual texture assets so we can load them last

  if (import_textures_) {
    AppendUnreferencedTextures(scene, texes_to_load);
  }

  // Stable subasset identities.
  model_import::SubassetKeys const subasset_keys{
    .prefab = "prefab",
    .meshes = bake_into_single_mesh
                ? std::vector<std::string>{"mesh:baked"}
                : MakeMeshGroupKeys(nodes, nodes_by_mesh, meshes_by_group),
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
      auto const it = std::ranges::lower_bound(material_settings_, key, {}, &MaterialSettingsEntry::key);
      auto const settings =
        it != material_settings_.end() && it->key == key
          ? it->settings
          : MaterialImportSettings{};
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
      auto const it = std::ranges::lower_bound(texture_settings_, key, {}, &TextureSettingsEntry::key);
      auto const settings =
        it != texture_settings_.end() && it->key == key
          ? it->settings
          : TextureImportSettings{
            .type = TextureImportType::kTexture2D,
            .allow_block_compression = true,
            .is_srgb = model_import::IsSrgb(texes_to_load[i].type),
            .generate_mips = true
          };
      new_settings_entries.emplace_back(key, settings);
      tex_settings.emplace_back(settings);
    }

    std::ranges::sort(new_settings_entries, {}, &TextureSettingsEntry::key);
    texture_settings_ = std::move(new_settings_entries);
  }

  auto const subasset_ids = model_import::SubassetIds::FromKeys(subasset_keys, src);

  // Now we have subasset ids so we can patch material->texture references
  ResolveTextureIds(mtl_assets, subasset_ids);

  // Import all textures

  std::vector<AssetImportResult> texture_import_results;

  if (import_textures_) {
    if (!BuildTextureResults(src, texes_to_load, tex_coverage_thresholds, tex_settings, subasset_ids,
      texture_import_results)) {
      return false;
    }
  }

  // Write subassets to output in package order

  if (bake_into_single_mesh) {
    results.emplace_back(CreateSingleEntityPrefab(ToUntypedSv(src.filename().stem().u8string()), imported_meshes.at(0),
      subasset_ids, mtl_slots_by_mesh.at(0)));
  } else {
    results.emplace_back(CreateHierarchyPrefab(ToUntypedSv(src.filename().stem().u8string()), imported_meshes, groups_by_node,
      mtl_slots_by_mesh, nodes, subasset_ids));
  }

  for (auto const [mesh_idx, mesh_data] : std::views::enumerate(imported_meshes)) {
    auto mesh_bytes = model_import::SerializeMesh(mesh_data);
    results.emplace_back(subasset_ids.GetMeshId(mesh_idx), GetCurrentPayloadVersion(AssetPackagePayloadKind::kMesh),
      AssetPackagePayloadKind::kMesh, rttr::type::get<Mesh>(), std::string{ToUntypedSv(src.stem().u8string())},
      std::move(mesh_bytes));
  }

  if (import_materials_) {
    BuildMaterialResults(mtl_assets, mtl_slots, subasset_ids, results);
  }

  if (import_textures_) {
    std::ranges::move(texture_import_results, std::back_inserter(results));
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
