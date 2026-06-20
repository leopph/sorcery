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
  selected_object_ptr_{std::addressof(selected_object_ptr)} {}


auto ResourceDB::Refresh() -> void {
  spdlog::debug("Starting resource database refresh.");

  std::map<ResourceId, ResourceEntry> new_id_to_entry;
  std::map<Guid, std::filesystem::path> new_guid_to_src_abs_path;
  std::map<Guid, std::filesystem::path> new_guid_to_res_abs_path;
  std::map<std::filesystem::path, Guid> new_src_abs_path_to_guid;

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

      if (!InternalImportResource(res_path_abs, new_guid_to_src_abs_path, new_guid_to_res_abs_path,
        new_src_abs_path_to_guid, new_id_to_entry, *importer, guid)) {
        spdlog::trace("Failed to import resource at file [{}]. Removing resource and meta files.",
          ToUntypedStdSv(res_path_abs.u8string()));
        cleanup_res_and_meta_files();
        continue;
      }

      spdlog::trace("Imported resource file [{}] with guid [{}] as new.", ToUntypedStdSv(res_path_abs.u8string()),
        new_src_abs_path_to_guid[res_path_abs].ToString());
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

        if (!InternalImportResource(res_path_abs, new_guid_to_src_abs_path, new_guid_to_res_abs_path,
          new_src_abs_path_to_guid, new_id_to_entry, *importer, guid)) {
          spdlog::trace(
            "Failed to reimport external resource at file [{}]. Removing resource, meta, and binary cache files.",
            ToUntypedStdSv(res_path_abs.u8string()));
          cleanup_res_meta_and_cache_files();
          continue;
        }
      }

      spdlog::trace("Storing binary cache path for external resource at file [{}] in resource database.",
        ToUntypedStdSv(res_path_abs.u8string()));
      new_guid_to_res_abs_path.emplace(guid, cache_file_path_abs);
    } else {
      spdlog::trace("Storing resource path for native resource at file [{}] in resource database.",
        ToUntypedStdSv(res_path_abs.u8string()));
      new_guid_to_res_abs_path.emplace(guid, res_path_abs);
    }

    spdlog::trace("Done processing resource file [{}] with guid [{}].", ToUntypedStdSv(res_path_abs.u8string()),
      guid.ToString());

    if (importer->IsNativeImporter()) {
      // Native import is basically no-op, we can use it to determine the type
      if (std::vector<ResourceImportResult> results; importer->Import(res_path_abs, results) && !results.empty()) {
        new_id_to_entry.emplace(ResourceId{guid, 0}, ResourceEntry{results[0].runtime_type, results[0].name});
      } else {
        spdlog::error("Failed to query native resource type from file [{}].", ToUntypedStdSv(res_path_abs.u8string()));
      }
    } else {
      if (auto const info{PeekBinaryResourcePackage(MakeExternalResourceBinaryPathAbs(guid))}) {
        for (std::size_t i{0}; i < info->entries.size(); ++i) {
          new_id_to_entry.emplace(ResourceId{guid, static_cast<int>(i)},
            ResourceEntry{info->entries[i].runtime_type, info->entries[i].name});
        }
      } else {
        spdlog::error("Failed to query resource type for external resource at file [{}].",
          ToUntypedStdSv(res_path_abs.u8string()));
      }
    }

    new_guid_to_src_abs_path.emplace(guid, res_path_abs);
    new_src_abs_path_to_guid.emplace(res_path_abs, guid);
  }

  spdlog::trace("Unloading removed resources.");

  // We unload resources that are no longer present in the current file system directory.
  for (auto const& guid : guid_to_src_abs_path_ | std::views::keys) {
    if (!new_guid_to_src_abs_path.contains(guid)) {
      ClearSelectionIfGuid(guid);
      UnloadResourcesFromFile(guid);
    }
  }

  spdlog::trace("Renaming moved resources.");

  // We rename loaded resources that have been moved in the file system
  for (auto const& [guid, pathAbs] : new_guid_to_src_abs_path) {
    if (auto const it{guid_to_src_abs_path_.find(guid)};
      it != std::end(guid_to_src_abs_path_) && it->second != pathAbs && App::Instance().GetResourceManager().
      IsLoaded(ResourceId{guid, 0})) {
      App::Instance().GetResourceManager().GetOrLoad(ResourceId{guid, 0})->SetName(pathAbs.stem().string());
    }
  }

  spdlog::trace("Updating resource database.");

  guid_to_src_abs_path_ = std::move(new_guid_to_src_abs_path);
  guid_to_load_abs_path_ = std::move(new_guid_to_res_abs_path);
  id_to_entry_ = std::move(new_id_to_entry);
  src_abs_path_to_guid_ = std::move(new_src_abs_path_to_guid);

  spdlog::trace("Updating resource mappings.");

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));

  spdlog::debug("Finished resource database refresh.");
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

  for (auto const& guid : guid_to_src_abs_path_ | std::views::keys) {
    UnloadResourcesFromFile(guid);
  }

  guid_to_src_abs_path_.clear();
  guid_to_load_abs_path_.clear();
  id_to_entry_.clear();
  src_abs_path_to_guid_.clear();

  Refresh();
}


auto ResourceDB::GetResourceDirectoryAbsolutePath() -> std::filesystem::path const& {
  return res_dir_abs_;
}


auto ResourceDB::CreateResource(std::unique_ptr<NativeResource>&& res,
                                std::filesystem::path const& target_path_res_dir_rel) -> ObserverPtr<NativeResource> {
  if (!res) {
    return nullptr;
  }

  if (!res->GetId().IsValid()) {
    res->SetId(ResourceId{Guid::Generate(), 0});
  }

  auto const res_node{res->Serialize()};
  auto const res_path_abs{res_dir_abs_ / target_path_res_dir_rel};
  std::ofstream out_res_stream{res_path_abs};
  YAML::Emitter res_emitter{out_res_stream};
  res_emitter << res_node;

  if (!WriteMeta(res_path_abs, res->GetId().GetGuid(), NativeResourceImporter{})) {
    return nullptr;
  }

  res->SetName(target_path_res_dir_rel.stem().string());

  guid_to_src_abs_path_.insert_or_assign(res->GetId().GetGuid(), res_path_abs);
  id_to_entry_.insert_or_assign(res->GetId(), ResourceEntry{rttr::type::get(res), res->GetName()});
  guid_to_load_abs_path_.insert_or_assign(res->GetId().GetGuid(), res_path_abs);
  src_abs_path_to_guid_.insert_or_assign(res_path_abs, res->GetId().GetGuid());

  auto const ret{App::Instance().GetResourceManager().Add(std::move(res))};
  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));
  return ret;
}


auto ResourceDB::SaveResource(NativeResource const& res) -> void {
  if (auto const it{guid_to_src_abs_path_.find(res.GetId().GetGuid())}; it != std::end(guid_to_src_abs_path_)) {
    std::ofstream outStream{it->second};
    YAML::Emitter emitter{outStream};
    emitter << res.Serialize();
  }
}


auto ResourceDB::ImportResource(std::filesystem::path const& res_path_res_dir_rel, ResourceImporter* importer) -> bool {
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

  if (!InternalImportResource(res_dir_abs_ / res_path_res_dir_rel, guid_to_src_abs_path_, guid_to_load_abs_path_,
    src_abs_path_to_guid_, id_to_entry_, *importer, guid)) {
    return false;
  }

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));
  return true;
}


auto ResourceDB::MoveResource(Guid const& guid, std::filesystem::path const& target_path_res_dir_rel) -> bool {
  auto const it{guid_to_src_abs_path_.find(guid)};

  if (it == std::end(guid_to_src_abs_path_)) {
    return false;
  }

  auto const srcPathAbs{it->second};
  auto const srcMetaPathAbs{MakeMetaPath(srcPathAbs)};
  auto const dstPathAbs{res_dir_abs_ / target_path_res_dir_rel};
  auto const dstMetaPathAbs{MakeMetaPath(dstPathAbs)};

  if (!exists(srcPathAbs) || !exists(srcMetaPathAbs) || exists(dstPathAbs) || exists(dstMetaPathAbs)) {
    return false;
  }

  rename(srcPathAbs, dstPathAbs);
  rename(srcMetaPathAbs, dstMetaPathAbs);
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
  UnloadResourcesFromFile(guid);

  if (auto const it{guid_to_src_abs_path_.find(guid)}; it != std::end(guid_to_src_abs_path_)) {
    std::filesystem::remove(it->second);
    std::filesystem::remove(MakeMetaPath(it->second));
    src_abs_path_to_guid_.erase(it->second);
    guid_to_src_abs_path_.erase(it);
  }

  guid_to_load_abs_path_.erase(guid);
  std::erase_if(id_to_entry_, [&guid](auto const& pair) { return pair.first.GetGuid() == guid; });

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));
}


auto ResourceDB::DeleteDirectory(std::filesystem::path const& path_res_dir_rel) -> bool {
  auto const pathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / path_res_dir_rel)};

  if (!exists(pathAbs) || !is_directory(pathAbs)) {
    return false;
  }

  std::vector<Guid> resourcesToDelete;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{pathAbs}) {
    if (auto const it{src_abs_path_to_guid_.find(entry.path())}; it != std::end(src_abs_path_to_guid_)) {
      resourcesToDelete.emplace_back(it->second);
    }
  }

  for (auto const& guid : resourcesToDelete) {
    DeleteResourceFile(guid);
  }

  remove_all(pathAbs);
  return true;
}


auto ResourceDB::IsSavedResource(NativeResource const& res) const -> bool {
  return guid_to_src_abs_path_.contains(res.GetId().GetGuid());
}


auto ResourceDB::PathToGuid(std::filesystem::path const& path_res_dir_rel) -> Guid {
  if (auto const it{src_abs_path_to_guid_.find(GetResourceDirectoryAbsolutePath() / path_res_dir_rel)};
    it != std::end(src_abs_path_to_guid_)) {
    return it->second;
  }

  return Guid::Invalid();
}


auto ResourceDB::GuidToPath(Guid const& guid) -> std::filesystem::path {
  if (auto const it{guid_to_src_abs_path_.find(guid)}; it != std::end(guid_to_src_abs_path_)) {
    return relative(it->second, GetResourceDirectoryAbsolutePath());
  }

  return {};
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

  return {};
}


auto ResourceDB::MakeMetaPath(std::filesystem::path const& path) -> std::filesystem::path {
  return std::filesystem::path{path} += kResourceMetaFileExt;
}


auto ResourceDB::IsMetaFile(std::filesystem::path const& path) -> bool {
  return path.extension() == kResourceMetaFileExt;
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

    ReflectionDeserializeFromYaml(importer_props_node, *importer_ptr);
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
                                        std::map<Guid, std::filesystem::path>& guid_to_src_abs_path,
                                        std::map<Guid, std::filesystem::path>& guid_to_res_abs_path,
                                        std::map<std::filesystem::path, Guid>& src_abs_path_to_guid,
                                        std::map<ResourceId, ResourceEntry>& id_to_entry, ResourceImporter& importer,
                                        Guid const& guid) const -> bool {
  if (!WriteMeta(res_path_abs, guid, importer)) {
    return false;
  }

  std::vector<ResourceImportResult> import_results;

  if (!importer.Import(res_path_abs, import_results)) {
    return false;
  }

  if (!importer.IsNativeImporter()) {
    if (!WriteBinaryResourcePackage(guid, import_results)) {
      return false;
    }
    guid_to_res_abs_path.insert_or_assign(guid, MakeExternalResourceBinaryPathAbs(guid));
  } else {
    guid_to_res_abs_path.insert_or_assign(guid, res_path_abs);
  }

  guid_to_src_abs_path.insert_or_assign(guid, res_path_abs);
  src_abs_path_to_guid.insert_or_assign(res_path_abs, guid);

  for (std::size_t i{0}; i < import_results.size(); ++i) {
    id_to_entry.insert_or_assign(ResourceId{guid, static_cast<int>(i)},
      ResourceEntry{import_results[i].runtime_type, import_results[i].name});
  }
  return true;
}


auto ResourceDB::CreateMappings() const noexcept -> std::pair<
  std::map<ResourceId, ResourceManager::ResourceDescription>, std::map<Guid, std::filesystem::path>> {
  std::map<ResourceId, ResourceManager::ResourceDescription> res_mappings;
  std::map<Guid, std::filesystem::path> file_mappings;

  for (auto const& [id, entry] : id_to_entry_) {
    auto const guid{id.GetGuid()};

    if (auto const it{guid_to_src_abs_path_.find(guid)}; it != std::end(guid_to_src_abs_path_)) {
      res_mappings.emplace(id, ResourceManager::ResourceDescription{entry.name, entry.type});
    } else {
      spdlog::error("Resource with ID [{}, {}] has no source path in the resource database.",
        guid.ToString(), id.GetIdxInFile());
    }
  }

  for (auto const& [guid, load_abs_path] : guid_to_load_abs_path_) {
    file_mappings.emplace(guid, load_abs_path);
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

  std::ofstream out_stream{MakeExternalResourceBinaryPathAbs(guid), std::ios::binary | std::ios::out | std::ios::trunc};

  if (!out_stream.is_open()) {
    return false;
  }

  out_stream.write(reinterpret_cast<char const*>(package_bytes->data()), std::ssize(*package_bytes));
  return true;
}


auto ResourceDB::UnloadResourcesFromFile(Guid const& guid) -> void {
  for (auto const& id : id_to_entry_ | std::views::keys) {
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
