#include "ResourceDB.hpp"

#include "app.hpp"
#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "resource_manager.hpp"
#include "Util.hpp"
#include "ResourceImporters/NativeResourceImporter.hpp"

#include "ExternalResource.hpp"

#include <fstream>
#include <ranges>
#include <utility>


namespace sorcery::mage {
ResourceDB::ResourceDB(Object*& selectedObjectPtr) :
  selected_object_ptr_{std::addressof(selectedObjectPtr)} {}


auto ResourceDB::Refresh() -> void {
  std::map<Guid, rttr::type> newGuidToType;
  std::map<Guid, std::filesystem::path> newGuidToSrcAbsPath;
  std::map<Guid, std::filesystem::path> newGuidToResAbsPath;
  std::map<std::filesystem::path, Guid> newSrcAbsPathToGuid;

  for (auto& entry : std::filesystem::recursive_directory_iterator{res_dir_abs_}) {
    if (!entry.exists() || entry.is_directory()) {
      continue;
    }

    if (IsMetaFile(entry.path())) {
      auto const resPathAbs{std::filesystem::path{entry.path()}.replace_extension()};

      // If it's an orphaned meta file, we remove it
      if (!exists(resPathAbs)) {
        remove(entry.path());
        continue;
      }

      Guid guid;
      std::unique_ptr<ResourceImporter> importer;

      // If we couldn't read the meta file (e.g. it's corrupted) we attempt to create a new one
      if (!ReadMeta(resPathAbs, &guid, &importer)) {
        importer = CreateNewImporterForResourceFile(entry.path());
        guid = Guid::Generate();

        // If we couldn't find an importer or we for some reason couldn't write the new meta, we just remove the files
        if (!importer || WriteMeta(resPathAbs, guid, *importer)) {
          remove(resPathAbs);
          remove(entry.path());
          continue;
        }
      }

      // If its an external resource, we check for the processed binary
      if (!importer->IsNativeImporter()) {
        auto const cacheFilePathAbs{MakeExternalResourceBinaryPathAbs(guid)};

        // If it's out of date we attempt to recreate it
        if (!exists(cacheFilePathAbs) || last_write_time(resPathAbs) > last_write_time(cacheFilePathAbs) ||
            last_write_time(entry.path()) > last_write_time(cacheFilePathAbs)) {
          // If we fail, we just remove the the files
          if (!InternalImportResource(resPathAbs, newGuidToSrcAbsPath, newGuidToResAbsPath, newSrcAbsPathToGuid,
            newGuidToType, *importer, guid)) {
            remove(resPathAbs);
            remove(entry.path());
            continue;
          }
        }

        // In case the resource is external, the processed binary is the path to load
        newGuidToResAbsPath.emplace(guid, cacheFilePathAbs);
      } else {
        // If the resource is native, the source is the path to load
        newGuidToResAbsPath.emplace(guid, resPathAbs);
      }

      newGuidToSrcAbsPath.emplace(guid, resPathAbs);
      newGuidToType.emplace(guid, importer->GetImportedType(resPathAbs));
      newSrcAbsPathToGuid.emplace(resPathAbs, guid);

      continue;
    }

    // If we find a file that is not a meta file, we attempt to import it as a resource
    if (auto const metaPathAbs{MakeMetaPath(entry.path())}; !exists(metaPathAbs)) {
      if (auto const importer{CreateNewImporterForResourceFile(entry.path())}) {
        if (auto const guid{Guid::Generate()}; InternalImportResource(entry.path(), newGuidToSrcAbsPath,
          newGuidToResAbsPath, newSrcAbsPathToGuid, newGuidToType, *importer, guid)) {
          continue;
        }
      }

      // If we couldn't import, we just remove the files
      remove(entry.path());
      remove(metaPathAbs);
    }
  }

  // We unload resources that are no longer present in the current file system directory.
  for (auto const& guid : guid_to_src_abs_path_ | std::views::keys) {
    if (!newGuidToSrcAbsPath.contains(guid) && App::Instance().GetResourceManager().IsLoaded(ResourceId{guid, 0})) {
      if (*selected_object_ptr_ == App::Instance().GetResourceManager().GetOrLoad(ResourceId{guid, 0})) {
        *selected_object_ptr_ = nullptr;
      }
      App::Instance().GetResourceManager().Unload(ResourceId{guid, 0});
    }
  }

  // We rename loaded resources that have been moved in the file system
  for (auto const& [guid, pathAbs] : newGuidToSrcAbsPath) {
    if (auto const it{guid_to_src_abs_path_.find(guid)};
      it != std::end(guid_to_src_abs_path_) && it->second != pathAbs && App::Instance().GetResourceManager().
      IsLoaded(ResourceId{guid, 0})) {
      App::Instance().GetResourceManager().GetOrLoad(ResourceId{guid, 0})->SetName(pathAbs.stem().string());
    }
  }

  guid_to_src_abs_path_ = std::move(newGuidToSrcAbsPath);
  guid_to_load_abs_path_ = std::move(newGuidToResAbsPath);
  guid_to_type_ = std::move(newGuidToType);
  src_abs_path_to_guid_ = std::move(newSrcAbsPathToGuid);

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));
}


auto ResourceDB::ChangeProjectDir(std::filesystem::path const& projDirAbs) -> void {
  if (!exists(projDirAbs)) {
    create_directory(projDirAbs);
  }

  res_dir_abs_ = projDirAbs / kResourceDirProjRel;
  cache_dir_abs_ = projDirAbs / kCacheDirProjRel;

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
  guid_to_type_.clear();
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

  auto const resNode{res->Serialize()};
  auto const resPathAbs{res_dir_abs_ / target_path_res_dir_rel};
  std::ofstream outResStream{resPathAbs};
  YAML::Emitter resEmitter{outResStream};
  resEmitter << resNode;

  if (!WriteMeta(resPathAbs, res->GetId().GetGuid(), NativeResourceImporter{})) {
    return nullptr;
  }

  res->SetName(target_path_res_dir_rel.stem().string());

  guid_to_src_abs_path_.insert_or_assign(res->GetId().GetGuid(), resPathAbs);
  guid_to_type_.insert_or_assign(res->GetId().GetGuid(), rttr::type::get(res));
  guid_to_load_abs_path_.insert_or_assign(res->GetId().GetGuid(), resPathAbs);
  src_abs_path_to_guid_.insert_or_assign(resPathAbs, res->GetId().GetGuid());

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


auto ResourceDB::ImportResource(std::filesystem::path const& resPathResDirRel, ResourceImporter* importer) -> bool {
  // Temporary to keep a potentially freshly created importer alive in this stack frame.
  // Use the observer ptr, do not access this directly.
  std::unique_ptr<ResourceImporter> ownedImporter;

  if (!importer) {
    // If we weren't passed an importer instance, we use a new default one.
    ownedImporter = CreateNewImporterForResourceFile(resPathResDirRel);
    importer = ownedImporter.get();

    if (!importer) {
      return false;
    }
  }

  auto guid{Guid::Invalid()};

  // If a meta file already exists for the resource, we attempt to reimport it and keep its Guid.
  if (ReadMeta(GetResourceDirectoryAbsolutePath() / resPathResDirRel, std::addressof(guid), nullptr) && App::Instance()
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

  if (!InternalImportResource(res_dir_abs_ / resPathResDirRel, guid_to_src_abs_path_, guid_to_load_abs_path_,
    src_abs_path_to_guid_, guid_to_type_, *importer, guid)) {
    return false;
  }

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));
  return true;
}


auto ResourceDB::MoveResource(Guid const& guid, std::filesystem::path const& targetPathResDirRel) -> bool {
  auto const it{guid_to_src_abs_path_.find(guid)};

  if (it == std::end(guid_to_src_abs_path_)) {
    return false;
  }

  auto const srcPathAbs{it->second};
  auto const srcMetaPathAbs{MakeMetaPath(srcPathAbs)};
  auto const dstPathAbs{res_dir_abs_ / targetPathResDirRel};
  auto const dstMetaPathAbs{MakeMetaPath(dstPathAbs)};

  if (!exists(srcPathAbs) || !exists(srcMetaPathAbs) || exists(dstPathAbs) || exists(dstMetaPathAbs)) {
    return false;
  }

  rename(srcPathAbs, dstPathAbs);
  rename(srcMetaPathAbs, dstMetaPathAbs);
  Refresh();

  return true;
}


auto ResourceDB::MoveDirectory(std::filesystem::path const& srcPathResDirRel,
                               std::filesystem::path const& dstPathResDirRel) -> bool {
  auto const srcPathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / srcPathResDirRel)};
  auto const dstPathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / dstPathResDirRel)};

  if (!exists(srcPathAbs) || exists(dstPathAbs) || !is_directory(srcPathAbs) || equivalent(srcPathAbs,
        GetResourceDirectoryAbsolutePath())) {
    return false;
  }

  rename(srcPathAbs, dstPathAbs);
  Refresh();

  return true;
}


auto ResourceDB::DeleteResource(Guid const& guid) -> void {
  App::Instance().GetResourceManager().Unload(ResourceId{guid, 0});

  if (auto const it{guid_to_src_abs_path_.find(guid)}; it != std::end(guid_to_src_abs_path_)) {
    std::filesystem::remove(it->second);
    std::filesystem::remove(MakeMetaPath(it->second));
    src_abs_path_to_guid_.erase(it->second);
    guid_to_src_abs_path_.erase(it);
  }

  guid_to_load_abs_path_.erase(guid);
  guid_to_type_.erase(guid);

  auto [res_mappings, file_mappings]{CreateMappings()};
  App::Instance().GetResourceManager().UpdateMappings(std::move(res_mappings), std::move(file_mappings));
}


auto ResourceDB::DeleteDirectory(std::filesystem::path const& pathResDirRel) -> bool {
  auto const pathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / pathResDirRel)};

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
    DeleteResource(guid);
  }

  remove_all(pathAbs);
  return true;
}


auto ResourceDB::IsSavedResource(NativeResource const& res) const -> bool {
  return guid_to_src_abs_path_.contains(res.GetId().GetGuid());
}


auto ResourceDB::PathToGuid(std::filesystem::path const& pathResDirRel) -> Guid {
  if (auto const it{src_abs_path_to_guid_.find(GetResourceDirectoryAbsolutePath() / pathResDirRel)};
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


auto ResourceDB::MakeMetaPath(std::filesystem::path const& path) -> std::filesystem::path {
  return std::filesystem::path{path} += kResourceMetaFileExt;
}


auto ResourceDB::IsMetaFile(std::filesystem::path const& path) -> bool {
  return path.extension() == kResourceMetaFileExt;
}


auto ResourceDB::ReadMeta(std::filesystem::path const& resPathAbs, Guid* const guid,
                          std::unique_ptr<ResourceImporter>* const importer) noexcept -> bool {
  auto const metaPathAbs{MakeMetaPath(resPathAbs)};

  if (!exists(metaPathAbs)) {
    return false;
  }

  auto const metaNode{YAML::LoadFile(metaPathAbs.string())};

  if (!metaNode) {
    return false;
  }

  if (guid) {
    auto const guidNode{metaNode["guid"]};

    if (!guidNode) {
      return false;
    }

    auto const parsedGuid{Guid::Parse(guidNode.as<std::string>(""))};

    if (!parsedGuid.IsValid()) {
      return false;
    }

    *guid = parsedGuid;
  }

  if (importer) {
    auto const importerNode{metaNode["importer"]};

    if (!importerNode) {
      return false;
    }

    auto const importerTypeNode{importerNode["type"]};

    if (!importerTypeNode) {
      return false;
    }

    auto const importerType{rttr::type::get_by_name(importerTypeNode.as<std::string>(""))};

    if (!importerType.is_valid()) {
      return false;
    }

    auto importerVariant{importerType.create()};

    if (!importerVariant.is_valid()) {
      return false;
    }

    auto conversionSuccess{false};
    auto const importerPtr{importerVariant.convert<ResourceImporter*>(&conversionSuccess)};

    if (!conversionSuccess) {
      return false;
    }

    auto const importerPropsNode{importerNode["properties"]};

    if (!importerPropsNode) {
      return false;
    }

    ReflectionDeserializeFromYaml(importerPropsNode, *importerPtr);
    importer->reset(importerPtr);
  }

  return true;
}


auto ResourceDB::WriteMeta(std::filesystem::path const& resPathAbs, Guid const& guid,
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

  auto const metaPathAbs{MakeMetaPath(resPathAbs)};

  std::ofstream outStream{metaPathAbs, std::ios::out | std::ios::trunc};

  if (!outStream.is_open()) {
    return false;
  }

  YAML::Emitter metaEmitter{outStream};
  metaEmitter << metaNode;
  return true;
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


auto ResourceDB::InternalImportResource(std::filesystem::path const& res_path_abs,
                                        std::map<Guid, std::filesystem::path>& guidToSrcAbsPath,
                                        std::map<Guid, std::filesystem::path>& guidToResAbsPath,
                                        std::map<std::filesystem::path, Guid>& srcAbsPathToGuid,
                                        std::map<Guid, rttr::type>& guidToType, ResourceImporter& importer,
                                        Guid const& guid) const -> bool {
  if (!WriteMeta(res_path_abs, guid, importer)) {
    return false;
  }

  std::vector<std::byte> resBytes;
  ExternalResourceCategory categ;

  if (!importer.Import(res_path_abs, resBytes, categ)) {
    return false;
  }

  if (!importer.IsNativeImporter()) {
    if (!WriteExternalResourceBinary(guid, categ, resBytes)) {
      return false;
    }
    guidToResAbsPath.insert_or_assign(guid, MakeExternalResourceBinaryPathAbs(guid));
  } else {
    guidToResAbsPath.insert_or_assign(guid, res_path_abs);
  }

  guidToSrcAbsPath.insert_or_assign(guid, res_path_abs);
  guidToType.insert_or_assign(guid, importer.GetImportedType(res_path_abs));
  srcAbsPathToGuid.insert_or_assign(res_path_abs, guid);
  return true;
}


auto ResourceDB::CreateMappings() const noexcept -> std::pair<
  std::map<ResourceId, ResourceManager::ResourceDescription>, std::map<Guid, std::filesystem::path>> {
  std::map<ResourceId, ResourceManager::ResourceDescription> res_mappings;
  std::map<Guid, std::filesystem::path> file_mappings;

  for (auto const& [guid, src_abs_path] : guid_to_src_abs_path_) {
    res_mappings.emplace(std::piecewise_construct, std::forward_as_tuple(ResourceId{guid, 0}),
      std::forward_as_tuple(src_abs_path.stem().string(), guid_to_type_.at(guid)));

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


auto ResourceDB::WriteExternalResourceBinary(Guid const& guid, ExternalResourceCategory const categ,
                                             std::span<std::byte const> const resBytes) const noexcept -> bool {
  if (!guid.IsValid()) {
    return false;
  }

  std::vector<std::byte> fileBytes;
  PackExternalResource(categ, resBytes, fileBytes);

  if (!exists(cache_dir_abs_)) {
    create_directory(cache_dir_abs_);
  }

  std::ofstream outStream{MakeExternalResourceBinaryPathAbs(guid), std::ios::binary | std::ios::out | std::ios::trunc};

  if (!outStream.is_open()) {
    return false;
  }

  outStream.write(reinterpret_cast<char*>(fileBytes.data()), std::ssize(fileBytes));
  return true;
}
}
