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

  std::map<ResourceId, rttr::type> new_id_to_type;
  std::map<Guid, std::filesystem::path> new_guid_to_src_abs_path;
  std::map<Guid, std::filesystem::path> new_guid_to_res_abs_path;
  std::map<std::filesystem::path, Guid> new_src_abs_path_to_guid;

  spdlog::trace("Scanning resource directory at [{}].", ToUntypedStdSv(res_dir_abs_.u8string()));

  for (auto& entry : std::filesystem::recursive_directory_iterator{res_dir_abs_}) {
    /*
     * We can break down the refresh procedure to a couple of distinct steps.
     * If the entry is a directory, we can safely skip it and the loop will recursively enter it at a later iteration.
     * If the entry is a resource file, the following steps are to be done:
     *   1) Check if there is a meta file for this resource. If not, try to import it as a new resource. If this fails, remove both files.
     *   2) If there is a meta file, we can skip it since it will be handled when we encounter the meta file in the iteration.
     * If the entry is a meta file, we have to go through the following steps:
     *   1) Make sure it's readable, otherwise try to reimport the associated resource. If this fails too, remove both files.
     *   2) If it's an external resource, check if the binary cache is up to date. If not, try to reimport. If this fails, remove all three files.
     *   3) If everything is fine, we store the resource in the new mappings.
     */

    if (!entry.exists() || entry.is_directory()) {
      continue;
    }

    if (IsMetaFile(entry.path())) {
      spdlog::trace("Found meta file at [{}].", ToUntypedStdSv(entry.path().u8string()));

      auto const res_path_abs{std::filesystem::path{entry.path()}.replace_extension()};

      // If it's an orphaned meta file, we remove it
      if (!exists(res_path_abs)) {
        spdlog::trace("Removing orphaned meta file at [{}].", ToUntypedStdSv(entry.path().u8string()));
        remove(entry.path());
        continue;
      }

      Guid guid;
      std::unique_ptr<ResourceImporter> importer;

      auto const cleanup_meta_and_res_files{
        [&] {
          remove(res_path_abs);
          remove(entry.path());
        }
      };

      // If we couldn't read the meta file (e.g. it's corrupted) we attempt to create a new one
      if (!ReadMeta(res_path_abs, &guid, &importer)) {
        spdlog::trace("Failed to read meta file at [{}]. Attempting to recreate.",
          ToUntypedStdSv(entry.path().u8string()));

        importer = CreateNewImporterForResourceFile(res_path_abs);
        guid = Guid::Generate();

        //  If we couldn't find an importer, we clean the files up
        if (!importer) {
          spdlog::trace("Couldn't find importer for resource file at [{}]. Removing along with meta.",
            ToUntypedStdSv(res_path_abs.u8string()));
          cleanup_meta_and_res_files();
          continue;
        }

        // If we for some reason couldn't write the new meta, we clean the files up
        if (!WriteMeta(res_path_abs, guid, *importer)) {
          spdlog::trace("Failed to write new meta file for resource file at [{}]. Removing both files.",
            ToUntypedStdSv(entry.path().u8string()));
          cleanup_meta_and_res_files();
          continue;
        }
      }

      // If its an external resource, we check for the processed binary
      if (!importer->IsNativeImporter()) {
        spdlog::trace("Resource at file [{}] is not a native resource. Checking for binary cache.",
          ToUntypedStdSv(entry.path().u8string()));

        auto const cache_file_path_abs{MakeExternalResourceBinaryPathAbs(guid)};

        // If it's out of date we attempt to recreate it
        if (!exists(cache_file_path_abs) || last_write_time(res_path_abs) > last_write_time(cache_file_path_abs) ||
            last_write_time(entry.path()) > last_write_time(cache_file_path_abs)) {
          spdlog::trace("Binary cache for external resource at file [{}] is out of date. Attempting to recreate.",
            ToUntypedStdSv(entry.path().u8string()));

          // If we fail, we just remove the the files
          if (!InternalImportResource(res_path_abs, new_guid_to_src_abs_path, new_guid_to_res_abs_path,
            new_src_abs_path_to_guid,
            new_id_to_type, *importer, guid)) {
            spdlog::trace(
              "Failed to recreate binary cache for external resource at file [{}]. Removing resource, meta, and binary cache files.",
              ToUntypedStdSv(entry.path().u8string()));
            cleanup_meta_and_res_files();
            remove(cache_file_path_abs);
            continue;
          }
        }

        spdlog::trace("Storing binary cache path for external resource at file [{}] in resource database.",
          ToUntypedStdSv(entry.path().u8string()));

        // In case the resource is external, the processed binary is the path to load
        new_guid_to_res_abs_path.emplace(guid, cache_file_path_abs);
      } else {
        spdlog::trace("Storing resource path for native resource at file [{}] in resource database.",
          ToUntypedStdSv(entry.path().u8string()));
        // If the resource is native, the source is the path to load
        new_guid_to_res_abs_path.emplace(guid, res_path_abs);
      }

      spdlog::trace("Imported resource file [{}] with guid [{}].",
        ToUntypedStdSv(entry.path().u8string()), guid.ToString());

      new_guid_to_src_abs_path.emplace(guid, res_path_abs);
      new_id_to_type.emplace(guid, imported_type);
      new_src_abs_path_to_guid.emplace(res_path_abs, guid);

      continue;
    }

    // If we find a file that is not a meta file, we attempt to import it as a resource
    spdlog::trace("Found resource file at [{}]. Attempting to import.", ToUntypedStdSv(entry.path().u8string()));

    // If there is no meta file for this resource, we attempt to import it as a new resource.
    if (auto const meta_path_abs{MakeMetaPath(entry.path())}; !exists(meta_path_abs)) {
      spdlog::trace("Couldn't find meta file for resource at [{}]. Attempting to import as new.",
        ToUntypedStdSv(entry.path().u8string()));

      auto const importer{CreateNewImporterForResourceFile(entry.path())};

      auto const cleanup_meta_and_res_files{
        [&] {
          remove(entry.path());
          remove(meta_path_abs);
        }
      };

      // If we couldn't find an importer, we clean the files up
      if (!importer) {
        spdlog::trace("Couldn't find importer for resource file at [{}]. Removing along with meta.",
          ToUntypedStdSv(entry.path().u8string()));
        cleanup_meta_and_res_files();
        continue;
      }

      auto const guid{Guid::Generate()};

      // If we couldn't import, we clean the files up
      if (!InternalImportResource(entry.path(), new_guid_to_src_abs_path, new_guid_to_res_abs_path,
        new_src_abs_path_to_guid, new_id_to_type, *importer, guid)) {
        spdlog::trace("Failed to import resource at file [{}]. Removing both files.",
          ToUntypedStdSv(entry.path().u8string()));
        cleanup_meta_and_res_files();
        continue;
      }

      spdlog::trace("Imported resource file [{}] with guid [{}] as new.",
        ToUntypedStdSv(entry.path().u8string()), new_src_abs_path_to_guid[entry.path()].ToString());
      continue;
    }

    spdlog::trace("Resource file at [{}] already has a meta file. Nothing to do.",
      ToUntypedStdSv(entry.path().u8string()));
  }

  spdlog::trace("Unloading removed resources.");

  // We unload resources that are no longer present in the current file system directory.
  for (auto const& guid : guid_to_src_abs_path_ | std::views::keys) {
    if (!new_guid_to_src_abs_path.contains(guid) && App::Instance().GetResourceManager().
                                                                    IsLoaded(ResourceId{guid, 0})) {
      if (*selected_object_ptr_ == App::Instance().GetResourceManager().GetOrLoad(ResourceId{guid, 0})) {
        *selected_object_ptr_ = nullptr;
      }
      App::Instance().GetResourceManager().Unload(ResourceId{guid, 0});
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
  id_to_type_ = std::move(new_id_to_type);
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
    App::Instance().GetResourceManager().Unload(ResourceId{guid, 0});
  }

  guid_to_src_abs_path_.clear();
  guid_to_load_abs_path_.clear();
  id_to_type_.clear();
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
  id_to_type_.insert_or_assign(res->GetId(), rttr::type::get(res));
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
  if (ReadMeta(GetResourceDirectoryAbsolutePath() / res_path_res_dir_rel, std::addressof(guid), nullptr) &&
      App::Instance()
      .GetResourceManager().IsLoaded(ResourceId{guid, 0})) {
    if (*selected_object_ptr_ == App::Instance().GetResourceManager().GetOrLoad(ResourceId{guid, 0})) {
      *selected_object_ptr_ = nullptr;
    }

    App::Instance().GetResourceManager().Unload(ResourceId{guid, 0});
  }

  // If there is no meta file, we proceed with a regular import.
  if (!guid.IsValid()) {
    guid = Guid::Generate();
  }

  if (!InternalImportResource(res_dir_abs_ / res_path_res_dir_rel, guid_to_src_abs_path_, guid_to_load_abs_path_,
    src_abs_path_to_guid_, id_to_type_, *importer, guid)) {
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
  App::Instance().GetResourceManager().Unload(ResourceId{guid, 0});

  if (auto const it{guid_to_src_abs_path_.find(guid)}; it != std::end(guid_to_src_abs_path_)) {
    std::filesystem::remove(it->second);
    std::filesystem::remove(MakeMetaPath(it->second));
    src_abs_path_to_guid_.erase(it->second);
    guid_to_src_abs_path_.erase(it);
  }

  guid_to_load_abs_path_.erase(guid);
  id_to_type_.erase(guid);

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
                                        std::map<ResourceId, rttr::type>& id_to_type, ResourceImporter& importer,
                                        Guid const& guid) const -> bool {
  if (!WriteMeta(res_path_abs, guid, importer)) {
    return false;
  }

  // TODO implement multi-resource support
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
    id_to_type.insert_or_assign(ResourceId{guid, static_cast<int>(i)}, import_results[i].runtime_type);
  }
  return true;
}


auto ResourceDB::CreateMappings() const noexcept -> std::pair<
  std::map<ResourceId, ResourceManager::ResourceDescription>, std::map<Guid, std::filesystem::path>> {
  std::map<ResourceId, ResourceManager::ResourceDescription> res_mappings;
  std::map<Guid, std::filesystem::path> file_mappings;

  for (auto const& [guid, src_abs_path] : guid_to_src_abs_path_) {
    res_mappings.emplace(std::piecewise_construct, std::forward_as_tuple(ResourceId{guid, 0}),
      std::forward_as_tuple(src_abs_path.stem().string(), id_to_type_.at(guid)));

    if (auto const load_abs_path{guid_to_load_abs_path_.find(guid)};
      load_abs_path != std::end(guid_to_load_abs_path_)) {
      file_mappings.emplace(guid, load_abs_path->second);
    }
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
}
