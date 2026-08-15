#include "ResourceDB.hpp"

#include <fstream>
#include <ranges>
#include <utility>

#include <spdlog/spdlog.h>

#include "app.hpp"
#include "Reflection.hpp"
#include "resource_manager.hpp"
#include "Util.hpp"
#include "ResourceImporters/native_resource_importer.hpp"


namespace sorcery::mage {
ResourceDB::ResourceDB(Object*& selected_object_ptr) :
  OnDatabaseChanged{db_changed_},
  selected_object_ptr_{std::addressof(selected_object_ptr)} {}


auto ResourceDB::Refresh() -> void {
  spdlog::debug("Starting resource database refresh.");

  std::map<std::filesystem::path, Guid> new_guid_by_src_abs_path;
  std::map<Guid, ResourceFileInfo> new_res_file_info_by_guid;
  std::map<ResourceId, ResourceInfo> new_res_info_by_id;

  spdlog::trace("Scanning resource directory at [{}].", ToUntypedStdSv(res_dir_abs_.u8string()));

  std::vector<std::filesystem::path> meta_file_paths;
  std::vector<std::filesystem::path> res_file_paths;

  // It is undefined whether recursive_directory_iterator sees filesystem changes after its creation
  // It is safer to first collect all the files and then process them in a second loop

  for (auto& entry : std::filesystem::recursive_directory_iterator{res_dir_abs_}) {
    if (!entry.exists() || entry.is_directory()) {
      continue;
    }

    if (IsMetaFile(entry.path())) {
      meta_file_paths.emplace_back(entry.path());
      spdlog::trace("Found meta file at [{}].", ToUntypedStdSv(entry.path().u8string()));
    } else {
      res_file_paths.emplace_back(entry.path());
      spdlog::trace("Found resource file at [{}].", ToUntypedStdSv(entry.path().u8string()));
    }
  }

  // Remove orphaned meta files first so we can focus on the resources files later.

  for (auto const& meta_path_abs : meta_file_paths) {
    if (auto const res_path_abs{std::filesystem::path{meta_path_abs}.replace_extension()}; !exists(res_path_abs)) {
      spdlog::trace("Removing orphaned meta file at [{}].", ToUntypedStdSv(meta_path_abs.u8string()));
      remove(meta_path_abs);
    }
  }

  // Process resource files.

  for (auto const& res_path_abs : res_file_paths) {
    auto const meta_path_abs{MakeMetaPath(res_path_abs)};

    Guid guid;
    std::unique_ptr<ResourceImporter> importer;

    auto const cleanup_res_and_meta_files{
      [&] {
        remove(meta_path_abs);
        remove(res_path_abs);
      }
    };

    // If there is no meta file, or we couldn't read it, we attempt to reimport the resource as new.

    if (!exists(meta_path_abs) || !ReadMeta(res_path_abs, &guid, &importer)) {
      spdlog::trace("Couldn't read meta file for resource at [{}]. Attempting to import as new.",
        ToUntypedStdSv(res_path_abs.u8string()));

      importer = CreateNewImporterForResourceFile(res_path_abs);

      if (!importer) {
        spdlog::trace("Couldn't create importer for resource file at [{}]. Removing resource and meta files.",
          ToUntypedStdSv(res_path_abs.u8string()));
        cleanup_res_and_meta_files();
        continue;
      }

      guid = Guid::Generate();

      if (!WriteMeta(res_path_abs, guid, *importer)) {
        spdlog::trace("Failed to write new meta file for resource file at [{}]. Removing resource and meta files.",
          ToUntypedStdSv(res_path_abs.u8string()));
        cleanup_res_and_meta_files();
        continue;
      }

      if (!InternalImportResource(res_path_abs, new_guid_by_src_abs_path, new_res_file_info_by_guid, new_res_info_by_id,
        *importer, guid)) {
        spdlog::trace("Failed to import resource at file [{}]. Removing resource and meta files.",
          ToUntypedStdSv(res_path_abs.u8string()));
        cleanup_res_and_meta_files();
        continue;
      }

      spdlog::trace("Imported resource file [{}] with guid [{}] as new.", ToUntypedStdSv(res_path_abs.u8string()),
        new_guid_by_src_abs_path[res_path_abs].ToString());

      // We reimported the source, the database is up-to-date, nothing else to do for this resource file.
      continue;
    }

    // Check if there is a binary cache for this resource and if it's up to date. If not, we attempt to reimport it.

    if (!importer->IsNativeImporter()) {
      spdlog::trace("Resource at file [{}] is not a native resource. Checking for binary cache.",
        ToUntypedStdSv(res_path_abs.u8string()));

      auto const cache_file_path_abs{MakeExternalResourceBinaryPathAbs(guid)};

      auto const cleanup_res_meta_and_cache_files{
        [&] {
          cleanup_res_and_meta_files();
          remove(cache_file_path_abs);
        }
      };

      if (!exists(cache_file_path_abs) || last_write_time(res_path_abs) > last_write_time(cache_file_path_abs) ||
          last_write_time(meta_path_abs) > last_write_time(cache_file_path_abs)) {
        spdlog::trace("Binary cache for external resource at file [{}] is out of date. Attempting to reimport.",
          ToUntypedStdSv(res_path_abs.u8string()));

        if (!InternalImportResource(res_path_abs, new_guid_by_src_abs_path, new_res_file_info_by_guid,
          new_res_info_by_id, *importer, guid)) {
          spdlog::trace(
            "Failed to reimport external resource at file [{}]. Removing resource, meta, and binary cache files.",
            ToUntypedStdSv(res_path_abs.u8string()));
          cleanup_res_meta_and_cache_files();
          continue;
        }

        spdlog::trace("Reimported resource file [{}] with guid [{}] due to out-of-date binary cache.",
          ToUntypedStdSv(res_path_abs.u8string()), new_guid_by_src_abs_path[res_path_abs].ToString());

        // We reimported the source, the database is up-to-date, nothing else to do for this resource file.
        continue;
      }
    }

    // The meta file was read successfully, the resource is up-to-date, we can just store the information in the database.

    std::optional<ResourcePackageInfo> package_info;

    if (!importer->IsNativeImporter()) {
      package_info = PeekBinaryResourcePackage(MakeExternalResourceBinaryPathAbs(guid));

      if (!package_info) {
        spdlog::error("Failed to peek binary resource package for external resource at file [{}].",
          ToUntypedStdSv(res_path_abs.u8string()));
        cleanup_res_and_meta_files();
        continue;
      }
    }

    ResourceFileInfo const file_info{
      .guid = guid,
      .src_path_res_dir_rel = relative(res_path_abs, res_dir_abs_),
      .load_path_abs = importer->IsNativeImporter() ? res_path_abs : MakeExternalResourceBinaryPathAbs(guid),
      .subresource_count = importer->IsNativeImporter() ? 1 : static_cast<int>(package_info->entries.size()),
      .is_native_resource = importer->IsNativeImporter()
    };

    // Store guid-source path mapping in the new database.
    new_guid_by_src_abs_path.insert_or_assign(res_path_abs, guid);

    // Store the resource file information in the new database.
    new_res_file_info_by_guid.insert_or_assign(guid, file_info);

    if (importer->IsNativeImporter()) {
      // Native import is basically no-op, we can use it to determine the type
      if (std::vector<ResourceImportResult> results; importer->Import(res_path_abs, results) && !results.empty()) {
        ResourceId const res_id{guid, 0};
        // Store the resource information in the new database.
        new_res_info_by_id.emplace(res_id, ResourceInfo{
          .id = res_id,
          .type = results[0].runtime_type,
          .name = results[0].name
        });
      } else {
        spdlog::error("Failed to query native resource type from file [{}].", ToUntypedStdSv(res_path_abs.u8string()));
      }
    } else {
      for (std::size_t i{0}; i < package_info->entries.size(); ++i) {
        // Store the resource information in the new database.
        new_res_info_by_id.insert_or_assign(ResourceId{guid, static_cast<int>(i)},
          ResourceInfo{
            .id = ResourceId{guid, static_cast<int>(i)},
            .type = package_info->entries[i].runtime_type,
            .name = package_info->entries[i].name
          });
      }
    }

    spdlog::trace("Done processing resource file [{}] with guid [{}].", ToUntypedStdSv(res_path_abs.u8string()),
      guid.ToString());
  }

  spdlog::trace("Unloading removed resources.");

  // We unload resources that are no longer present in the current file system directory.
  for (auto const& guid : res_file_info_by_guid_ | std::views::keys) {
    if (!new_res_file_info_by_guid.contains(guid)) {
      ClearSelectionIfGuid(guid);
      UnloadResourcesFromFile(guid);
    }
  }

  spdlog::trace("Renaming moved resources.");

  // We rename loaded resources that have been moved in the file system
  for (auto const& [guid, resource_file_info] : new_res_file_info_by_guid) {
    if (auto const it{new_res_file_info_by_guid.find(guid)};
      it != std::end(new_res_file_info_by_guid) &&
      it->second.src_path_res_dir_rel != resource_file_info.src_path_res_dir_rel &&
      App::Instance().GetResourceManager().IsLoaded(ResourceId{guid, 0})) {
      App::Instance().GetResourceManager().GetOrLoad(ResourceId{guid, 0})->SetName(
        resource_file_info.src_path_res_dir_rel.stem().string());
    }
  }

  spdlog::trace("Updating resource database.");

  res_file_info_by_guid_ = std::move(new_res_file_info_by_guid);
  res_info_by_id_ = std::move(new_res_info_by_id);
  guid_by_src_abs_path_ = std::move(new_guid_by_src_abs_path);

  spdlog::trace("Updating resource mappings.");

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));

  spdlog::debug("Finished resource database refresh.");

  db_changed_.invoke();
}


auto ResourceDB::ChangeProjectDir(std::filesystem::path const& proj_dir_abs) -> void {
  if (!exists(proj_dir_abs)) {
    create_directory(proj_dir_abs);
  }

  res_dir_abs_ = proj_dir_abs / kResourceDirProjRel;
  cache_dir_abs_ = proj_dir_abs / kCacheDirProjRel;

  if (!exists(res_dir_abs_)) {
    create_directory(res_dir_abs_);
  }

  if (!exists(cache_dir_abs_)) {
    create_directory(cache_dir_abs_);
  }

  for (auto const& guid : res_file_info_by_guid_ | std::views::keys) {
    UnloadResourcesFromFile(guid);
  }

  res_file_info_by_guid_.clear();
  res_info_by_id_.clear();
  guid_by_src_abs_path_.clear();

  Refresh();
}


auto ResourceDB::GetResourceDirectoryAbsolutePath() const -> std::filesystem::path const& {
  return res_dir_abs_;
}


auto ResourceDB::SaveResourceToFile(
  std::unique_ptr<NativeResource>&& res,
  std::filesystem::path const& target_path_res_dir_rel)
  -> ObserverPtr<NativeResource> {
  if (!res) {
    return nullptr;
  }

  // If the resource doesn't have a valid ResourceId yet, we generate a new one.
  if (!res->GetId().IsValid()) {
    res->SetId(ResourceId{Guid::Generate(), 0});
  }

  // The serialized byte content of the resource
  auto const res_bytes{res->Serialize()};

  // Write resource bytes to file
  auto const res_path_abs{res_dir_abs_ / target_path_res_dir_rel};
  std::ofstream out_res_stream{res_path_abs};
  YAML::Emitter res_emitter{out_res_stream};
  res_emitter << res_bytes;

  // Write meta file
  if (!WriteMeta(res_path_abs, res->GetId().GetGuid(), NativeResourceImporter{})) {
    return nullptr;
  }

  // Update resource name
  res->SetName(target_path_res_dir_rel.stem().string());

  ResourceFileInfo const file_info{
    .guid = res->GetId().GetGuid(),
    .src_path_res_dir_rel = target_path_res_dir_rel,
    .load_path_abs = res_path_abs,
    .subresource_count = 1,
    .is_native_resource = true
  };

  // Update resource file record
  res_file_info_by_guid_.insert_or_assign(res->GetId().GetGuid(), file_info);
  // Update resource record
  res_info_by_id_.insert_or_assign(res->GetId(), ResourceInfo{
    .id = res->GetId(), .type = rttr::type::get(res), .name = res->GetName()
  });
  // Update guid-source path mapping
  guid_by_src_abs_path_.insert_or_assign(res_path_abs, res->GetId().GetGuid());

  // Transfer resource ownership to resource manager
  auto const ret{App::Instance().GetResourceManager().Add(std::move(res))};

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));

  db_changed_.invoke();

  return ret;
}


auto ResourceDB::SaveResourceToFile(NativeResource const& res) -> void {
  if (auto const it{res_file_info_by_guid_.find(res.GetId().GetGuid())}; it != std::end(res_file_info_by_guid_)) {
    std::ofstream out_stream{res_dir_abs_ / it->second.src_path_res_dir_rel};
    YAML::Emitter emitter{out_stream};
    emitter << res.Serialize();
  }
}


auto ResourceDB::ImportResourceFile(std::filesystem::path const& res_path_res_dir_rel,
                                    ResourceImporter* importer) -> bool {
  // Temporary to keep a potentially freshly created importer alive in this stack frame.
  // Use the observer ptr, do not access this directly.
  std::unique_ptr<ResourceImporter> ownedImporter;

  if (!importer) {
    // If we weren't passed an importer instance, we use a new default one.
    ownedImporter = CreateNewImporterForResourceFile(res_path_res_dir_rel);
    importer = ownedImporter.get();

    if (!importer) {
      return false;
    }
  }

  auto guid{Guid::Invalid()};

  // If a meta file already exists for the resource, we attempt to reimport it and keep its Guid.
  if (ReadMeta(GetResourceDirectoryAbsolutePath() / res_path_res_dir_rel, std::addressof(guid), nullptr)) {
    ClearSelectionIfGuid(guid);
    UnloadResourcesFromFile(guid);
  }

  // If there is no meta file, we proceed with a regular import.
  if (!guid.IsValid()) {
    guid = Guid::Generate();
  }

  if (!InternalImportResource(res_dir_abs_ / res_path_res_dir_rel, guid_by_src_abs_path_, res_file_info_by_guid_,
    res_info_by_id_, *importer, guid)) {
    return false;
  }

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));

  db_changed_.invoke();

  return true;
}


auto ResourceDB::MoveResourceFile(Guid const& guid, std::filesystem::path const& target_path_res_dir_rel) -> bool {
  auto const it{res_file_info_by_guid_.find(guid)};

  if (it == std::end(res_file_info_by_guid_)) {
    return false;
  }

  auto const src_path_abs{res_dir_abs_ / it->second.src_path_res_dir_rel};
  auto const src_meta_path_abs{MakeMetaPath(src_path_abs)};
  auto const dst_path_abs{res_dir_abs_ / target_path_res_dir_rel};
  auto const dst_meta_path_abs{MakeMetaPath(dst_path_abs)};

  if (!exists(src_path_abs) || !exists(src_meta_path_abs) || exists(dst_path_abs) || exists(dst_meta_path_abs)) {
    return false;
  }

  rename(src_path_abs, dst_path_abs);
  rename(src_meta_path_abs, dst_meta_path_abs);
  Refresh();

  return true;
}


auto ResourceDB::MoveDirectory(std::filesystem::path const& src_path_res_dir_rel,
                               std::filesystem::path const& dst_path_res_dir_rel) -> bool {
  auto const srcPathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / src_path_res_dir_rel)};
  auto const dstPathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / dst_path_res_dir_rel)};

  if (!exists(srcPathAbs) || exists(dstPathAbs) || !is_directory(srcPathAbs) || equivalent(srcPathAbs,
        GetResourceDirectoryAbsolutePath())) {
    return false;
  }

  rename(srcPathAbs, dstPathAbs);
  Refresh();

  return true;
}


auto ResourceDB::DeleteResourceFile(Guid const& guid) -> void {
  // Unload all resources associated with the resource file
  UnloadResourcesFromFile(guid);

  if (auto const it{res_file_info_by_guid_.find(guid)}; it != std::end(res_file_info_by_guid_)) {
    auto const src_path_abs{res_dir_abs_ / it->second.src_path_res_dir_rel};
    // Erase source file
    std::filesystem::remove(src_path_abs);
    // Erase meta file
    std::filesystem::remove(MakeMetaPath(src_path_abs));
    // Erase load file
    std::filesystem::remove(it->second.load_path_abs);
    // Erase guid-source path mapping
    guid_by_src_abs_path_.erase(src_path_abs);
  }

  // Erase resource file record
  res_file_info_by_guid_.erase(guid);

  // Erase resource info records for all subresources of the resource file
  std::erase_if(res_info_by_id_, [&guid](auto const& pair) { return pair.first.GetGuid() == guid; });

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));

  db_changed_.invoke();
}


auto ResourceDB::DeleteDirectory(std::filesystem::path const& path_res_dir_rel) -> bool {
  auto const pathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / path_res_dir_rel)};

  if (!exists(pathAbs) || !is_directory(pathAbs)) {
    return false;
  }

  std::vector<Guid> resourcesToDelete;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{pathAbs}) {
    if (auto const it{guid_by_src_abs_path_.find(entry.path())}; it != std::end(guid_by_src_abs_path_)) {
      resourcesToDelete.emplace_back(it->second);
    }
  }

  for (auto const& guid : resourcesToDelete) {
    DeleteResourceFile(guid);
  }

  remove_all(pathAbs);

  db_changed_.invoke();

  return true;
}


auto ResourceDB::IsSavedResource(NativeResource const& res) const -> bool {
  return res_file_info_by_guid_.contains(res.GetId().GetGuid());
}


auto ResourceDB::PathToGuid(std::filesystem::path const& path_res_dir_rel) const -> Guid {
  if (auto const it{guid_by_src_abs_path_.find(GetResourceDirectoryAbsolutePath() / path_res_dir_rel)};
    it != std::end(guid_by_src_abs_path_)) {
    return it->second;
  }

  return Guid::Invalid();
}


auto ResourceDB::GuidToPath(Guid const& guid) const -> std::filesystem::path {
  if (auto const it{res_file_info_by_guid_.find(guid)}; it != std::end(res_file_info_by_guid_)) {
    return relative(it->second.load_path_abs, GetResourceDirectoryAbsolutePath());
  }

  return {};
}


auto ResourceDB::GetFileInfo(Guid const& guid) const -> ObserverPtr<ResourceFileInfo const> {
  if (auto const it{res_file_info_by_guid_.find(guid)}; it != std::end(res_file_info_by_guid_)) {
    return ObserverPtr{&it->second};
  }

  return nullptr;
}


auto ResourceDB::GetResourceInfo(ResourceId const& id) const -> ObserverPtr<ResourceInfo const> {
  if (auto const it{res_info_by_id_.find(id)}; it != std::end(res_info_by_id_)) {
    return ObserverPtr{&it->second};
  }

  return nullptr;
}


auto ResourceDB::GetResourcesInFile(Guid const& guid, std::vector<ObserverPtr<ResourceInfo const>>& out) const -> void {
  out.clear();

  for (auto const& [id, info] : res_info_by_id_) {
    if (id.GetGuid() == guid) {
      out.emplace_back(ObserverPtr{&info});
    }
  }
}


auto ResourceDB::GetImporterForResourceFile(
  std::filesystem::path const& res_path_abs) noexcept -> std::unique_ptr<ResourceImporter> {
  if (std::unique_ptr<ResourceImporter> importer;
    ReadMeta(res_path_abs, nullptr, &importer)) {
    return importer;
  }

  return nullptr;
}


auto ResourceDB::CreateNewImporterForResourceFile(
  std::filesystem::path const& path) -> std::unique_ptr<ResourceImporter> {
  for (auto const& importerType : rttr::type::get<ResourceImporter>().get_derived_classes()) {
    auto importerVariant{importerType.create()};
    std::unique_ptr<ResourceImporter> importer{importerVariant.get_value<ResourceImporter*>()};

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


auto ResourceDB::IsMetaFile(std::filesystem::path const& path) -> bool {
  return path.extension() == kResourceMetaFileExt;
}


auto ResourceDB::MakeMetaPath(std::filesystem::path const& path) -> std::filesystem::path {
  return std::filesystem::path{path} += kResourceMetaFileExt;
}


auto ResourceDB::ReadMeta(std::filesystem::path const& res_path_abs, Guid* const guid,
                          std::unique_ptr<ResourceImporter>* const importer) noexcept -> bool {
  auto const metaPathAbs{MakeMetaPath(res_path_abs)};

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
    auto const importer_ptr{importer_variant.convert<ResourceImporter*>(&conversion_success)};

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


auto ResourceDB::WriteMeta(std::filesystem::path const& res_path_abs, Guid const& guid,
                           ResourceImporter const& importer) noexcept -> bool {
  if (!guid.IsValid()) {
    return false;
  }

  auto const importerType{rttr::type::get(importer)};

  if (!importerType.is_valid()) {
    return false;
  }

  YAML::Node importerNode;
  importerNode["type"] = importerType.get_name().to_string();
  importerNode["properties"] = ReflectionSerializeToYaml(importer);

  YAML::Node metaNode;
  metaNode["guid"] = guid;
  metaNode["importer"] = importerNode;

  auto const metaPathAbs{MakeMetaPath(res_path_abs)};

  std::ofstream outStream{metaPathAbs, std::ios::out | std::ios::trunc};

  if (!outStream.is_open()) {
    return false;
  }

  YAML::Emitter metaEmitter{outStream};
  metaEmitter << metaNode;
  return true;
}


auto ResourceDB::InternalImportResource(std::filesystem::path const& res_path_abs,
                                        std::map<std::filesystem::path, Guid>& guid_by_src_abs_path,
                                        std::map<Guid, ResourceFileInfo>& res_file_info_by_guid,
                                        std::map<ResourceId, ResourceInfo>& res_info_by_id, ResourceImporter& importer,
                                        Guid const& guid) const -> bool {
  std::vector<ResourceImportResult> import_results;

  if (!importer.Import(res_path_abs, import_results)) {
    return false;
  }

  // First import, then write meta, because importers can mutate their internal state during import, which may affect the meta file content.
  if (!WriteMeta(res_path_abs, guid, importer)) {
    return false;
  }

  ResourceFileInfo const file_info{
    .guid = guid,
    .src_path_res_dir_rel = relative(res_path_abs, res_dir_abs_),
    .load_path_abs = importer.IsNativeImporter() ? res_path_abs : MakeExternalResourceBinaryPathAbs(guid),
    .subresource_count = clamp_cast<int>(import_results.size()),
    .is_native_resource = importer.IsNativeImporter()
  };

  if (!importer.IsNativeImporter()) {
    if (!WriteBinaryResourcePackage(guid, import_results)) {
      return false;
    }
  }

  guid_by_src_abs_path.insert_or_assign(res_path_abs, guid);
  res_file_info_by_guid.insert_or_assign(guid, file_info);

  for (std::size_t i{0}; i < import_results.size(); ++i) {
    ResourceId const res_id{guid, static_cast<int>(i)};
    res_info_by_id.insert_or_assign(res_id, ResourceInfo{
      .id = res_id,
      .type = import_results[i].runtime_type,
      .name = import_results[i].name
    });
  }
  return true;
}


auto ResourceDB::CreateMappings() const noexcept -> std::pair<
  std::map<ResourceId, ResourceManager::ResourceDescription>, std::map<Guid, std::filesystem::path>> {
  std::map<ResourceId, ResourceManager::ResourceDescription> res_mappings;
  std::map<Guid, std::filesystem::path> file_mappings;

  for (auto const& [id, entry] : res_info_by_id_) {
    auto const guid{id.GetGuid()};

    if (auto const it{res_file_info_by_guid_.find(guid)}; it != std::end(res_file_info_by_guid_)) {
      res_mappings.emplace(id, ResourceManager::ResourceDescription{entry.name, entry.type});
    } else {
      spdlog::error("Resource with ID [{}, {}] has no source path in the resource database.",
        guid.ToString(), id.GetIdxInFile());
    }
  }

  for (auto const& [guid, file_info] : res_file_info_by_guid_) {
    file_mappings.emplace(guid, file_info.load_path_abs);
  }

  return std::make_pair(std::move(res_mappings), std::move(file_mappings));
}


auto ResourceDB::MakeExternalResourceBinaryPathAbs(Guid const& guid) const noexcept -> std::filesystem::path {
  return cache_dir_abs_ / static_cast<std::string>(guid) += ResourceManager::EXTERNAL_RESOURCE_EXT;
}


auto ResourceDB::WriteBinaryResourcePackage(
  Guid const& guid,
  std::span<ResourceImportResult const> const imports
) const noexcept -> bool {
  if (!guid.IsValid()) {
    return false;
  }

  auto const package_bytes{PackBinaryResourcePackage(imports)};

  if (!package_bytes) {
    return false;
  }

  if (!exists(cache_dir_abs_)) {
    create_directory(cache_dir_abs_);
  }

  std::ofstream out_stream{
    MakeExternalResourceBinaryPathAbs(guid), std::ios::binary | std::ios::out | std::ios::trunc
  };

  if (!out_stream.is_open()) {
    return false;
  }

  out_stream.write(reinterpret_cast<char const*>(package_bytes->data()), std::ssize(*package_bytes));
  return true;
}


auto ResourceDB::UnloadResourcesFromFile(Guid const& guid) -> void {
  for (auto const& id : res_info_by_id_ | std::views::keys) {
    if (id.GetGuid() == guid) {
      App::Instance().GetResourceManager().Unload(id);
    }
  }
}


auto ResourceDB::ClearSelectionIfGuid(Guid const& guid) const -> void {
  if (*selected_object_ptr_) {
    if (auto const res{dynamic_cast<Resource*>(*selected_object_ptr_)}; res && res->GetId().GetGuid() == guid) {
      *selected_object_ptr_ = nullptr;
    }
  }
}
}
