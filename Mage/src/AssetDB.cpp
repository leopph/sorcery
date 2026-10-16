#include "AssetDB.hpp"

#include <fstream>
#include <ranges>
#include <utility>

#include <spdlog/spdlog.h>

#include "AssetImporters/native_asset_importer.hpp"
#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/reflection.hpp"
#include "sorcery/util.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/assets/asset_manager.hpp"


namespace sorcery::mage {
AssetDB::AssetDB(ObjectPtr<Object>& selected_object_ptr) :
  OnDatabaseChanged{db_changed_},
  selected_object_ptr_{std::addressof(selected_object_ptr)} {}


auto AssetDB::Refresh() -> void {
  spdlog::debug("Starting asset database refresh.");

  std::map<std::filesystem::path, Guid> new_guid_by_src_abs_path;
  std::map<Guid, AssetFileInfo> new_asset_file_info_by_guid;
  std::map<AssetId, AssetInfo> new_asset_info_by_id;

  spdlog::trace("Scanning asset directory at [{}].", ToUntypedSv(asset_dir_abs_.u8string()));

  std::vector<std::filesystem::path> meta_file_paths;
  std::vector<std::filesystem::path> asset_file_paths;

  // It is undefined whether recursive_directory_iterator sees filesystem changes after its creation
  // It is safer to first collect all the files and then process them in a second loop

  for (auto& entry : std::filesystem::recursive_directory_iterator{asset_dir_abs_}) {
    if (!entry.exists() || entry.is_directory()) {
      continue;
    }

    if (IsMetaFile(entry.path())) {
      meta_file_paths.emplace_back(entry.path());
      spdlog::trace("Found meta file at [{}].", ToUntypedSv(entry.path().u8string()));
    } else {
      asset_file_paths.emplace_back(entry.path());
      spdlog::trace("Found asset file at [{}].", ToUntypedSv(entry.path().u8string()));
    }
  }

  // Remove orphaned meta files first so we can focus on the assets files later.

  for (auto const& meta_path_abs : meta_file_paths) {
    if (auto const asset_path_abs{std::filesystem::path{meta_path_abs}.replace_extension()}; !exists(asset_path_abs)) {
      spdlog::trace("Removing orphaned meta file at [{}].", ToUntypedSv(meta_path_abs.u8string()));
      remove(meta_path_abs);
    }
  }

  // Process asset files.

  for (auto const& asset_path_abs : asset_file_paths) {
    auto const meta_path_abs{MakeMetaPath(asset_path_abs)};

    Guid guid;
    std::unique_ptr<AssetImporter> importer;

    auto const cleanup_asset_and_meta_files{
      [&] {
        remove(meta_path_abs);
        remove(asset_path_abs);
      }
    };

    // If there is no meta file, or we couldn't read it, we attempt to reimport the asset as new.

    if (!exists(meta_path_abs) || !ReadMeta(asset_path_abs, &guid, &importer)) {
      spdlog::trace("Couldn't read meta file for asset at [{}]. Attempting to import as new.",
        ToUntypedSv(asset_path_abs.u8string()));

      importer = CreateNewImporterForAssetFile(asset_path_abs);

      if (!importer) {
        spdlog::trace("Couldn't create importer for asset file at [{}]. Removing asset and meta files.",
          ToUntypedSv(asset_path_abs.u8string()));
        cleanup_asset_and_meta_files();
        continue;
      }

      guid = Guid::Generate();

      if (!WriteMeta(asset_path_abs, guid, *importer)) {
        spdlog::trace("Failed to write new meta file for asset file at [{}]. Removing asset and meta files.",
          ToUntypedSv(asset_path_abs.u8string()));
        cleanup_asset_and_meta_files();
        continue;
      }

      if (!InternalImportAsset(asset_path_abs, new_guid_by_src_abs_path, new_asset_file_info_by_guid, new_asset_info_by_id,
        *importer, guid)) {
        spdlog::trace("Failed to import asset at file [{}]. Removing asset and meta files.",
          ToUntypedSv(asset_path_abs.u8string()));
        cleanup_asset_and_meta_files();
        continue;
      }

      spdlog::trace("Imported asset file [{}] with guid [{}] as new.", ToUntypedSv(asset_path_abs.u8string()),
        new_guid_by_src_abs_path[asset_path_abs].ToString());

      // We reimported the source, the database is up-to-date, nothing else to do for this asset file.
      continue;
    }

    // Check if there is a binary cache for this asset and if it's up to date. If not, we attempt to reimport it.

    if (!importer->IsNativeImporter()) {
      spdlog::trace("Asset at file [{}] is not a native asset. Checking for binary cache.",
        ToUntypedSv(asset_path_abs.u8string()));

      auto const cache_file_path_abs{MakeExternalAssetBinaryPathAbs(guid)};

      auto const cleanup_asset_meta_and_cache_files{
        [&] {
          cleanup_asset_and_meta_files();
          remove(cache_file_path_abs);
        }
      };

      if (!exists(cache_file_path_abs) || last_write_time(asset_path_abs) > last_write_time(cache_file_path_abs) ||
          last_write_time(meta_path_abs) > last_write_time(cache_file_path_abs)) {
        spdlog::trace("Binary cache for external asset at file [{}] is out of date. Attempting to reimport.",
          ToUntypedSv(asset_path_abs.u8string()));

        if (!InternalImportAsset(asset_path_abs, new_guid_by_src_abs_path, new_asset_file_info_by_guid,
          new_asset_info_by_id, *importer, guid)) {
          spdlog::trace(
            "Failed to reimport external asset at file [{}]. Removing asset, meta, and binary cache files.",
            ToUntypedSv(asset_path_abs.u8string()));
          cleanup_asset_meta_and_cache_files();
          continue;
        }

        spdlog::trace("Reimported asset file [{}] with guid [{}] due to out-of-date binary cache.",
          ToUntypedSv(asset_path_abs.u8string()), new_guid_by_src_abs_path[asset_path_abs].ToString());

        // We reimported the source, the database is up-to-date, nothing else to do for this asset file.
        continue;
      }
    }

    // The meta file was read successfully, the asset is up-to-date, we can just store the information in the database.

    std::optional<AssetPackageInfo> package_info;

    if (!importer->IsNativeImporter()) {
      package_info = PeekBinaryAssetPackage(MakeExternalAssetBinaryPathAbs(guid));

      if (!package_info) {
        spdlog::error("Failed to peek binary asset package for external asset at file [{}].",
          ToUntypedSv(asset_path_abs.u8string()));
        cleanup_asset_and_meta_files();
        continue;
      }
    }

    AssetFileInfo const file_info{
      .guid = guid,
      .src_path_asset_dir_rel = relative(asset_path_abs, asset_dir_abs_),
      .load_path_abs = importer->IsNativeImporter() ? asset_path_abs : MakeExternalAssetBinaryPathAbs(guid),
      .subasset_count = importer->IsNativeImporter() ? 1 : static_cast<int>(package_info->entries.size()),
      .is_native_asset = importer->IsNativeImporter()
    };

    // Store guid-source path mapping in the new database.
    new_guid_by_src_abs_path.insert_or_assign(asset_path_abs, guid);

    // Store the asset file information in the new database.
    new_asset_file_info_by_guid.insert_or_assign(guid, file_info);

    if (importer->IsNativeImporter()) {
      // Native import is basically no-op, we can use it to determine the type
      if (std::vector<AssetImportResult> results; importer->Import(asset_path_abs, results) && !results.empty()) {
        AssetId const asset_id{guid, 0};
        // Store the asset information in the new database.
        new_asset_info_by_id.emplace(asset_id, AssetInfo{
          .id = asset_id,
          .type = results[0].runtime_type,
          .name = results[0].name
        });
      } else {
        spdlog::error("Failed to query native asset type from file [{}].", ToUntypedSv(asset_path_abs.u8string()));
      }
    } else {
      for (std::size_t i{0}; i < package_info->entries.size(); ++i) {
        // Store the asset information in the new database.
        new_asset_info_by_id.insert_or_assign(AssetId{guid, static_cast<int>(i)},
          AssetInfo{
            .id = AssetId{guid, static_cast<int>(i)},
            .type = package_info->entries[i].runtime_type,
            .name = package_info->entries[i].name
          });
      }
    }

    spdlog::trace("Done processing asset file [{}] with guid [{}].", ToUntypedSv(asset_path_abs.u8string()),
      guid.ToString());
  }

  spdlog::trace("Unloading removed assets.");

  // We unload assets that are no longer present in the current file system directory.
  for (auto const& guid : asset_file_info_by_guid_ | std::views::keys) {
    if (!new_asset_file_info_by_guid.contains(guid)) {
      ClearSelectionIfGuid(guid);
      UnloadAssetsFromFile(guid);
    }
  }

  spdlog::trace("Renaming moved assets.");

  // We rename loaded assets that have been moved in the file system
  for (auto const& [guid, asset_file_info] : new_asset_file_info_by_guid) {
    if (auto const it{new_asset_file_info_by_guid.find(guid)};
      it != std::end(new_asset_file_info_by_guid) &&
      it->second.src_path_asset_dir_rel != asset_file_info.src_path_asset_dir_rel &&
      App::Instance().GetAssetManager().IsLoaded(AssetId{guid, 0})) {
      App::Instance().GetAssetManager().Resolve(AssetId{guid, 0})->SetName(
        asset_file_info.src_path_asset_dir_rel.stem().string());
    }
  }

  spdlog::trace("Updating asset database.");

  asset_file_info_by_guid_ = std::move(new_asset_file_info_by_guid);
  asset_info_by_id_ = std::move(new_asset_info_by_id);
  guid_by_src_abs_path_ = std::move(new_guid_by_src_abs_path);

  spdlog::trace("Updating asset mappings.");

  auto [asset_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetAssetManager().UpdateMappings(std::move(asset_mappings), std::move(file_mappings));

  spdlog::debug("Finished asset database refresh.");

  db_changed_.invoke();
}


auto AssetDB::ChangeProjectDir(std::filesystem::path const& proj_dir_abs) -> void {
  if (!exists(proj_dir_abs)) {
    create_directory(proj_dir_abs);
  }

  asset_dir_abs_ = proj_dir_abs / kAssetDirProjRel;
  cache_dir_abs_ = proj_dir_abs / kCacheDirProjRel;

  if (!exists(asset_dir_abs_)) {
    create_directory(asset_dir_abs_);
  }

  if (!exists(cache_dir_abs_)) {
    create_directory(cache_dir_abs_);
  }

  for (auto const& guid : asset_file_info_by_guid_ | std::views::keys) {
    UnloadAssetsFromFile(guid);
  }

  asset_file_info_by_guid_.clear();
  asset_info_by_id_.clear();
  guid_by_src_abs_path_.clear();

  Refresh();
}


auto AssetDB::GetAssetDirectoryAbsolutePath() const -> std::filesystem::path const& {
  return asset_dir_abs_;
}


auto AssetDB::SaveAssetToFile(
  std::unique_ptr<NativeAsset>&& asset,
  std::filesystem::path const& target_path_asset_dir_rel)
  -> AssetRef<NativeAsset> {
  if (!asset) {
    return nullptr;
  }

  if (!IsAssetEditable(asset->GetAssetId())) {
    return nullptr;
  }

  // If the asset doesn't have a valid AssetId yet, we generate a new one.
  if (!asset->GetAssetId().IsValid()) {
    asset->SetAssetId(AssetId{Guid::Generate(), 0});
  }

  // The serialized byte content of the asset
  auto const asset_bytes{asset->Serialize()};

  // Write asset bytes to file
  auto const asset_path_abs{asset_dir_abs_ / target_path_asset_dir_rel};
  std::ofstream out_asset_stream{asset_path_abs};
  YAML::Emitter asset_emitter{out_asset_stream};
  asset_emitter << asset_bytes;

  // Write meta file
  if (!WriteMeta(asset_path_abs, asset->GetAssetId().GetGuid(), NativeAssetImporter{})) {
    return nullptr;
  }

  // Update asset name
  asset->SetName(target_path_asset_dir_rel.stem().string());

  AssetFileInfo const file_info{
    .guid = asset->GetAssetId().GetGuid(),
    .src_path_asset_dir_rel = target_path_asset_dir_rel,
    .load_path_abs = asset_path_abs,
    .subasset_count = 1,
    .is_native_asset = true
  };

  // Update asset file record
  asset_file_info_by_guid_.insert_or_assign(asset->GetAssetId().GetGuid(), file_info);
  // Update asset record
  asset_info_by_id_.insert_or_assign(asset->GetAssetId(), AssetInfo{
    .id = asset->GetAssetId(), .type = rttr::type::get(asset), .name = asset->GetName()
  });
  // Update guid-source path mapping
  guid_by_src_abs_path_.insert_or_assign(asset_path_abs, asset->GetAssetId().GetGuid());

  // Transfer asset ownership to asset manager
  auto const ret{App::Instance().GetAssetManager().Add(std::move(asset))};

  auto [asset_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetAssetManager().UpdateMappings(std::move(asset_mappings), std::move(file_mappings));

  db_changed_.invoke();

  return ret;
}


auto AssetDB::SaveAssetToFile(NativeAsset const& asset) -> void {
  if (!IsAssetEditable(asset.GetAssetId())) {
    return;
  }

  if (auto const it{asset_file_info_by_guid_.find(asset.GetAssetId().GetGuid())}; it != std::end(asset_file_info_by_guid_)) {
    std::ofstream out_stream{asset_dir_abs_ / it->second.src_path_asset_dir_rel};
    YAML::Emitter emitter{out_stream};
    emitter << asset.Serialize();
  }
}


auto AssetDB::ImportAssetFile(std::filesystem::path const& asset_path_asset_dir_rel,
                                    AssetImporter* importer) -> bool {
  // Temporary to keep a potentially freshly created importer alive in this stack frame.
  // Use the observer ptr, do not access this directly.
  std::unique_ptr<AssetImporter> ownedImporter;

  if (!importer) {
    // If we weren't passed an importer instance, we use a new default one.
    ownedImporter = CreateNewImporterForAssetFile(asset_path_asset_dir_rel);
    importer = ownedImporter.get();

    if (!importer) {
      return false;
    }
  }

  auto guid{Guid::Invalid()};

  // If a meta file already exists for the asset, we attempt to reimport it and keep its Guid.
  if (ReadMeta(GetAssetDirectoryAbsolutePath() / asset_path_asset_dir_rel, std::addressof(guid), nullptr)) {
    ClearSelectionIfGuid(guid);
    UnloadAssetsFromFile(guid);
  }

  // If there is no meta file, we proceed with a regular import.
  if (!guid.IsValid()) {
    guid = Guid::Generate();
  }

  if (!InternalImportAsset(asset_dir_abs_ / asset_path_asset_dir_rel, guid_by_src_abs_path_, asset_file_info_by_guid_,
    asset_info_by_id_, *importer, guid)) {
    return false;
  }

  auto [asset_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetAssetManager().UpdateMappings(std::move(asset_mappings), std::move(file_mappings));

  db_changed_.invoke();

  return true;
}


auto AssetDB::MoveAssetFile(Guid const& guid, std::filesystem::path const& target_path_asset_dir_rel) -> bool {
  auto const it{asset_file_info_by_guid_.find(guid)};

  if (it == std::end(asset_file_info_by_guid_)) {
    return false;
  }

  auto const src_path_abs{asset_dir_abs_ / it->second.src_path_asset_dir_rel};
  auto const src_meta_path_abs{MakeMetaPath(src_path_abs)};
  auto const dst_path_abs{asset_dir_abs_ / target_path_asset_dir_rel};
  auto const dst_meta_path_abs{MakeMetaPath(dst_path_abs)};

  if (!exists(src_path_abs) || !exists(src_meta_path_abs) || exists(dst_path_abs) || exists(dst_meta_path_abs)) {
    return false;
  }

  rename(src_path_abs, dst_path_abs);
  rename(src_meta_path_abs, dst_meta_path_abs);
  Refresh();

  return true;
}


auto AssetDB::MoveDirectory(std::filesystem::path const& src_path_asset_dir_rel,
                               std::filesystem::path const& dst_path_asset_dir_rel) -> bool {
  auto const srcPathAbs{weakly_canonical(GetAssetDirectoryAbsolutePath() / src_path_asset_dir_rel)};
  auto const dstPathAbs{weakly_canonical(GetAssetDirectoryAbsolutePath() / dst_path_asset_dir_rel)};

  if (!exists(srcPathAbs) || exists(dstPathAbs) || !is_directory(srcPathAbs) || equivalent(srcPathAbs,
        GetAssetDirectoryAbsolutePath())) {
    return false;
  }

  rename(srcPathAbs, dstPathAbs);
  Refresh();

  return true;
}


auto AssetDB::DeleteAssetFile(Guid const& guid) -> void {
  // Unload all assets associated with the asset file
  UnloadAssetsFromFile(guid);

  if (auto const it{asset_file_info_by_guid_.find(guid)}; it != std::end(asset_file_info_by_guid_)) {
    auto const src_path_abs{asset_dir_abs_ / it->second.src_path_asset_dir_rel};
    // Erase source file
    std::filesystem::remove(src_path_abs);
    // Erase meta file
    std::filesystem::remove(MakeMetaPath(src_path_abs));
    // Erase load file
    std::filesystem::remove(it->second.load_path_abs);
    // Erase guid-source path mapping
    guid_by_src_abs_path_.erase(src_path_abs);
  }

  // Erase asset file record
  asset_file_info_by_guid_.erase(guid);

  // Erase asset info records for all subassets of the asset file
  std::erase_if(asset_info_by_id_, [&guid](auto const& pair) { return pair.first.GetGuid() == guid; });

  auto [asset_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetAssetManager().UpdateMappings(std::move(asset_mappings), std::move(file_mappings));

  db_changed_.invoke();
}


auto AssetDB::DeleteDirectory(std::filesystem::path const& path_asset_dir_rel) -> bool {
  auto const pathAbs{weakly_canonical(GetAssetDirectoryAbsolutePath() / path_asset_dir_rel)};

  if (!exists(pathAbs) || !is_directory(pathAbs)) {
    return false;
  }

  std::vector<Guid> assetsToDelete;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{pathAbs}) {
    if (auto const it{guid_by_src_abs_path_.find(entry.path())}; it != std::end(guid_by_src_abs_path_)) {
      assetsToDelete.emplace_back(it->second);
    }
  }

  for (auto const& guid : assetsToDelete) {
    DeleteAssetFile(guid);
  }

  remove_all(pathAbs);

  db_changed_.invoke();

  return true;
}


auto AssetDB::IsSavedAsset(NativeAsset const& asset) const -> bool {
  return asset_file_info_by_guid_.contains(asset.GetAssetId().GetGuid());
}


auto AssetDB::PathToGuid(std::filesystem::path const& path_asset_dir_rel) const -> Guid {
  if (auto const it{guid_by_src_abs_path_.find(GetAssetDirectoryAbsolutePath() / path_asset_dir_rel)};
    it != std::end(guid_by_src_abs_path_)) {
    return it->second;
  }

  return Guid::Invalid();
}


auto AssetDB::GuidToPath(Guid const& guid) const -> std::filesystem::path {
  if (auto const it{asset_file_info_by_guid_.find(guid)}; it != std::end(asset_file_info_by_guid_)) {
    return it->second.src_path_asset_dir_rel;
  }

  return {};
}


auto AssetDB::GetFileInfo(Guid const& guid) const -> ObserverPtr<AssetFileInfo const> {
  if (auto const it{asset_file_info_by_guid_.find(guid)}; it != std::end(asset_file_info_by_guid_)) {
    return ObserverPtr{&it->second};
  }

  return nullptr;
}


auto AssetDB::GetAssetInfo(AssetId const& id) const -> ObserverPtr<AssetInfo const> {
  if (auto const it{asset_info_by_id_.find(id)}; it != std::end(asset_info_by_id_)) {
    return ObserverPtr{&it->second};
  }

  return nullptr;
}


auto AssetDB::GetAssetsInFile(Guid const& guid, std::vector<ObserverPtr<AssetInfo const>>& out) const -> void {
  out.clear();

  for (auto const& [id, info] : asset_info_by_id_) {
    if (id.GetGuid() == guid) {
      out.emplace_back(ObserverPtr{&info});
    }
  }
}


auto AssetDB::IsAssetFileEditable(Guid const& guid) const -> bool {
  auto const file_info{GetFileInfo(guid)};
  // If the file info is not found, we assume it is editable, as it may be a new asset that has not been saved yet.
  return !file_info || file_info->is_native_asset;
}


auto AssetDB::IsAssetEditable(AssetId const& id) const -> bool {
  return IsAssetFileEditable(id.GetGuid());
}


auto AssetDB::GetImporterForAssetFile(
  std::filesystem::path const& asset_path_abs) noexcept -> std::unique_ptr<AssetImporter> {
  if (std::unique_ptr<AssetImporter> importer;
    ReadMeta(asset_path_abs, nullptr, &importer)) {
    return importer;
  }

  return nullptr;
}


auto AssetDB::CreateNewImporterForAssetFile(
  std::filesystem::path const& path) -> std::unique_ptr<AssetImporter> {
  for (auto const& importerType : rttr::type::get<AssetImporter>().get_derived_classes()) {
    auto importerVariant{importerType.create()};
    std::unique_ptr<AssetImporter> importer{importerVariant.get_value<AssetImporter*>()};

    std::pmr::vector<std::string> supportedExtensions;
    importer->GetSupportedFileExtensions(supportedExtensions);

    for (auto const& ext : supportedExtensions) {
      if (ext == path.extension()) {
        return importer;
      }
    }
  }

  return nullptr;
}


auto AssetDB::IsMetaFile(std::filesystem::path const& path) -> bool {
  return path.extension() == kAssetMetaFileExt;
}


auto AssetDB::MakeMetaPath(std::filesystem::path const& path) -> std::filesystem::path {
  return std::filesystem::path{path} += kAssetMetaFileExt;
}


auto AssetDB::ReadMeta(std::filesystem::path const& asset_path_abs, Guid* const guid,
                          std::unique_ptr<AssetImporter>* const importer) noexcept -> bool {
  auto const metaPathAbs{MakeMetaPath(asset_path_abs)};

  if (!exists(metaPathAbs)) {
    return false;
  }

  auto const meta_node{YAML::LoadFile(metaPathAbs.string())};

  if (!meta_node || !meta_node.IsMap()) {
    return false;
  }

  if (guid) {
    auto const guid_node{meta_node["guid"]};

    if (!guid_node || !guid_node.IsScalar()) {
      return false;
    }

    auto const parsed_guid{Guid::Parse(guid_node.as<std::string>(""))};

    if (!parsed_guid.IsValid()) {
      return false;
    }

    *guid = parsed_guid;
  }

  if (importer) {
    auto const importer_node{meta_node["importer"]};

    if (!importer_node || !importer_node.IsMap()) {
      return false;
    }

    auto const importer_type_node{importer_node["type"]};

    if (!importer_type_node || !importer_type_node.IsScalar()) {
      return false;
    }

    auto const importer_type{rttr::type::get_by_name(importer_type_node.as<std::string>(""))};

    if (!importer_type.is_valid()) {
      return false;
    }

    auto importer_variant{importer_type.create()};

    if (!importer_variant.is_valid()) {
      return false;
    }

    auto conversion_success{false};
    auto const importer_ptr{importer_variant.convert<AssetImporter*>(&conversion_success)};

    if (!conversion_success) {
      return false;
    }

    auto const importer_props_node{importer_node["properties"]};

    if (!importer_props_node) {
      return false;
    }

    ReflectionDeserializeFromYaml(importer_props_node, *importer_ptr, {});
    importer->reset(importer_ptr);
  }

  return true;
}


auto AssetDB::WriteMeta(std::filesystem::path const& asset_path_abs, Guid const& guid,
                           AssetImporter const& importer) noexcept -> bool {
  if (!guid.IsValid()) {
    return false;
  }

  auto const importerType{rttr::type::get(importer)};

  if (!importerType.is_valid()) {
    return false;
  }

  YAML::Node importerNode;
  importerNode["type"] = importerType.get_name().to_string();
  importerNode["properties"] = ReflectionSerializeToYaml(importer, {});

  YAML::Node metaNode;
  metaNode["guid"] = guid;
  metaNode["importer"] = importerNode;

  auto const metaPathAbs{MakeMetaPath(asset_path_abs)};

  std::ofstream outStream{metaPathAbs, std::ios::out | std::ios::trunc};

  if (!outStream.is_open()) {
    return false;
  }

  YAML::Emitter metaEmitter{outStream};
  metaEmitter << metaNode;
  return true;
}


auto AssetDB::InternalImportAsset(std::filesystem::path const& asset_path_abs,
                                        std::map<std::filesystem::path, Guid>& guid_by_src_abs_path,
                                        std::map<Guid, AssetFileInfo>& asset_file_info_by_guid,
                                        std::map<AssetId, AssetInfo>& asset_info_by_id, AssetImporter& importer,
                                        Guid const& guid) const -> bool {
  std::vector<AssetImportResult> import_results;

  if (!importer.Import(asset_path_abs, import_results)) {
    return false;
  }

  // First import, then write meta, because importers can mutate their internal state during import, which may affect the meta file content.
  if (!WriteMeta(asset_path_abs, guid, importer)) {
    return false;
  }

  AssetFileInfo const file_info{
    .guid = guid,
    .src_path_asset_dir_rel = relative(asset_path_abs, asset_dir_abs_),
    .load_path_abs = importer.IsNativeImporter() ? asset_path_abs : MakeExternalAssetBinaryPathAbs(guid),
    .subasset_count = clamp_cast<int>(import_results.size()),
    .is_native_asset = importer.IsNativeImporter()
  };

  if (!importer.IsNativeImporter()) {
    if (!WriteBinaryAssetPackage(guid, import_results)) {
      return false;
    }
  }

  guid_by_src_abs_path.insert_or_assign(asset_path_abs, guid);
  asset_file_info_by_guid.insert_or_assign(guid, file_info);

  // Erase asset info instances since a reimport might contain less subassets than it did before
  std::erase_if(asset_info_by_id, [&guid](auto const& pair) {
    return pair.first.GetGuid() == guid;
  });

  for (std::size_t i{0}; i < import_results.size(); ++i) {
    AssetId const asset_id{guid, static_cast<int>(i)};
    asset_info_by_id.insert_or_assign(asset_id, AssetInfo{
      .id = asset_id,
      .type = import_results[i].runtime_type,
      .name = import_results[i].name
    });
  }
  return true;
}


auto AssetDB::CreateMappings() const noexcept -> std::pair<
  std::map<AssetId, AssetManager::AssetDescription>, std::map<Guid, std::filesystem::path>> {
  std::map<AssetId, AssetManager::AssetDescription> asset_mappings;
  std::map<Guid, std::filesystem::path> file_mappings;

  for (auto const& [id, entry] : asset_info_by_id_) {
    auto const guid{id.GetGuid()};

    if (auto const it{asset_file_info_by_guid_.find(guid)}; it != std::end(asset_file_info_by_guid_)) {
      asset_mappings.emplace(id, AssetManager::AssetDescription{entry.name, entry.type});
    } else {
      spdlog::error("Asset with ID [{}, {}] has no source path in the asset database.",
        guid.ToString(), id.GetIdxInFile());
    }
  }

  for (auto const& [guid, file_info] : asset_file_info_by_guid_) {
    file_mappings.emplace(guid, file_info.load_path_abs);
  }

  return std::make_pair(std::move(asset_mappings), std::move(file_mappings));
}


auto AssetDB::MakeExternalAssetBinaryPathAbs(Guid const& guid) const noexcept -> std::filesystem::path {
  return cache_dir_abs_ / static_cast<std::string>(guid) += AssetManager::EXTERNAL_ASSET_EXT;
}


auto AssetDB::WriteBinaryAssetPackage(
  Guid const& guid,
  std::span<AssetImportResult const> const imports
) const noexcept -> bool {
  if (!guid.IsValid()) {
    return false;
  }

  auto const package_bytes{PackBinaryAssetPackage(imports)};

  if (!package_bytes) {
    return false;
  }

  if (!exists(cache_dir_abs_)) {
    create_directory(cache_dir_abs_);
  }

  std::ofstream out_stream{
    MakeExternalAssetBinaryPathAbs(guid), std::ios::binary | std::ios::out | std::ios::trunc
  };

  if (!out_stream.is_open()) {
    return false;
  }

  out_stream.write(reinterpret_cast<char const*>(package_bytes->data()), std::ssize(*package_bytes));
  return true;
}


auto AssetDB::UnloadAssetsFromFile(Guid const& guid) -> void {
  for (auto const& id : asset_info_by_id_ | std::views::keys) {
    if (id.GetGuid() == guid) {
      App::Instance().GetAssetManager().Unload(id);
    }
  }
}


auto AssetDB::ClearSelectionIfGuid(Guid const& guid) const -> void {
  if (*selected_object_ptr_) {
    if (auto const asset{ReflCast<Asset>(*selected_object_ptr_)}; asset && asset->GetAssetId().GetGuid() == guid) {
      *selected_object_ptr_ = nullptr;
    }
  }
}
}
