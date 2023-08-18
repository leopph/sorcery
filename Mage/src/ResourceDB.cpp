#include "ResourceDB.hpp"

#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "ResourceManager.hpp"
#include "Util.hpp"
#include "ResourceImporters/NativeResourceImporter.hpp"

#include "ExternalResource.hpp"

#include <fstream>
#include <ranges>


namespace sorcery::mage {
auto ResourceDB::InternalImportResource(std::filesystem::path const& resPathResDirRel, std::map<Guid, std::filesystem::path>& guidToSrcAbsPath, std::map<Guid, std::filesystem::path>& guidToResAbsPath, std::map<std::filesystem::path, Guid>& srcAbsPathToGuid, std::map<Guid, rttr::type>& guidToType, ResourceImporter& importer, Guid const& guid) const -> bool {
  auto const resPathAbs{mResDirAbs / resPathResDirRel};

  if (!WriteMeta(resPathAbs, guid, importer)) {
    return false;
  }

  std::vector<std::byte> resBytes;
  ExternalResourceCategory categ;

  if (!importer.Import(resPathAbs, resBytes, categ)) {
    return false;
  }

  if (!importer.IsNativeImporter()) {
    if (!WriteExternalResourceBinary(guid, categ, resBytes)) {
      return false;
    }
    guidToResAbsPath.insert_or_assign(guid, GetExternalResourceBinaryPathAbs(guid));
  } else {
    guidToResAbsPath.insert_or_assign(guid, resPathAbs);
  }

  guidToSrcAbsPath.insert_or_assign(guid, resPathAbs);
  guidToType.insert_or_assign(guid, importer.GetImportedType(resPathAbs));
  srcAbsPathToGuid.insert_or_assign(resPathAbs, guid);
  return true;
}


auto ResourceDB::CreateMappings() const noexcept -> std::map<Guid, ResourceManager::ResourceDescription> {
  std::map<Guid, ResourceManager::ResourceDescription> mappings;
  std::ranges::transform(mGuidToResAbsPath, std::inserter(mappings, std::end(mappings)), [this](std::pair<Guid const, std::filesystem::path> const& pair) {
    return std::pair{pair.first, ResourceManager::ResourceDescription{pair.second, mGuidToSrcAbsPath.at(pair.first).stem().string(), mGuidToType.at(pair.first)}};
  });
  return mappings;
}


auto ResourceDB::GetExternalResourceBinaryPathAbs(Guid const& guid) const noexcept -> std::filesystem::path {
  return mCacheDirAbs / static_cast<std::string>(guid) += ResourceManager::EXTERNAL_RESOURCE_EXT;
}


auto ResourceDB::WriteExternalResourceBinary(Guid const& guid, ExternalResourceCategory const categ, std::span<std::byte const> const resBytes) const noexcept -> bool {
  if (!guid.IsValid()) {
    return false;
  }

  std::vector<std::byte> fileBytes;
  PackExternalResource(categ, resBytes, fileBytes);

  if (!exists(mCacheDirAbs)) {
    create_directory(mCacheDirAbs);
  }

  std::ofstream outStream{GetExternalResourceBinaryPathAbs(guid), std::ios::binary | std::ios::out | std::ios::trunc};

  if (!outStream.is_open()) {
    return false;
  }

  outStream.write(reinterpret_cast<char*>(fileBytes.data()), std::ssize(fileBytes));
  return true;
}


ResourceDB::ResourceDB(ObserverPtr<Object>& selectedObjectPtr) :
  mSelectedObjectPtr{std::addressof(selectedObjectPtr)} {}


auto ResourceDB::Refresh() -> void {
  std::map<Guid, rttr::type> newGuidToType;
  std::map<Guid, std::filesystem::path> newGuidToSrcAbsPath;
  std::map<Guid, std::filesystem::path> newGuidToResAbsPath;
  std::map<std::filesystem::path, Guid> newSrcAbsPathToGuid;

  for (auto& entry : std::filesystem::recursive_directory_iterator{mResDirAbs}) {
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
      if (!LoadMeta(resPathAbs, &guid, &importer)) {
        importer = GetNewImporterForResourceFile(entry.path());
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
        auto const cacheFilePathAbs{GetExternalResourceBinaryPathAbs(guid)};

        // If it's out of date we attempt to recreate it
        if (!exists(cacheFilePathAbs) || last_write_time(resPathAbs) > last_write_time(cacheFilePathAbs) || last_write_time(entry.path()) > last_write_time(cacheFilePathAbs)) {
          // If we fail, we just remove the the files
          if (!InternalImportResource(resPathAbs.lexically_relative(mResDirAbs), newGuidToSrcAbsPath, newGuidToResAbsPath, newSrcAbsPathToGuid, newGuidToType, *importer, guid)) {
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
    if (auto const metaPathAbs{GetMetaPath(entry.path())}; !exists(metaPathAbs)) {
      if (auto const importer{GetNewImporterForResourceFile(entry.path())}) {
        if (auto const guid{Guid::Generate()}; InternalImportResource(entry.path().lexically_relative(mResDirAbs), newGuidToSrcAbsPath, newGuidToResAbsPath, newSrcAbsPathToGuid, newGuidToType, *importer, guid)) {
          continue;
        }
      }

      // If we couldn't import, we just remove the files
      remove(entry.path());
      remove(metaPathAbs);
    }
  }

  // We delete resources that are no longer present in the current file system directory.
  // Because DeleteResource modifies mGuidToSrcAbsPath, we collect the to be deleted resources first and delete them in separate loop

  std::vector<Guid> resourcesToDelete;

  for (auto const& guid : mGuidToSrcAbsPath | std::views::keys) {
    if (!newGuidToSrcAbsPath.contains(guid)) {
      resourcesToDelete.emplace_back(guid);
    }
  }

  for (auto const& guid : resourcesToDelete) {
    if (*mSelectedObjectPtr && *mSelectedObjectPtr == gResourceManager.GetOrLoad(guid)) {
      *mSelectedObjectPtr = nullptr;
    }
    DeleteResource(guid); // TODO this deletes existing resources that had their GUIDs regenerated during the refresh
  }

  // We rename loaded resources that have been moved in the file system
  for (auto const& [guid, pathAbs] : newGuidToSrcAbsPath) {
    if (auto const it{mGuidToSrcAbsPath.find(guid)}; it != std::end(mGuidToSrcAbsPath) && it->second != pathAbs && gResourceManager.IsLoaded(guid)) {
      gResourceManager.GetOrLoad(guid)->SetName(pathAbs.stem().string());
    }
  }

  mGuidToSrcAbsPath = std::move(newGuidToSrcAbsPath);
  mGuidToResAbsPath = std::move(newGuidToResAbsPath);
  mGuidToType = std::move(newGuidToType);
  mSrcAbsPathToGuid = std::move(newSrcAbsPathToGuid);

  gResourceManager.UpdateMappings(CreateMappings());
}


auto ResourceDB::ChangeProjectDir(std::filesystem::path const& projDirAbs) -> void {
  mResDirAbs = projDirAbs / RESOURCE_DIR_PROJ_REL;
  mCacheDirAbs = projDirAbs / CACHE_DIR_PROJ_REL;

  if (!exists(mResDirAbs)) {
    create_directory(mResDirAbs);
  }

  if (!exists(mCacheDirAbs)) {
    create_directory(mCacheDirAbs);
  }

  for (auto const& guid : mGuidToSrcAbsPath | std::views::keys) {
    gResourceManager.Unload(guid);
  }

  mGuidToSrcAbsPath.clear();
  mGuidToResAbsPath.clear();
  mGuidToType.clear();
  mSrcAbsPathToGuid.clear();

  Refresh();
}


auto ResourceDB::GetResourceDirectoryAbsolutePath() -> std::filesystem::path const& {
  return mResDirAbs;
}


auto ResourceDB::CreateResource(NativeResource& res, std::filesystem::path const& targetPathResDirRel) -> bool {
  if (!res.GetGuid().IsValid()) {
    res.SetGuid(Guid::Generate());
  }

  auto const resNode{res.Serialize()};
  auto const resPathAbs{mResDirAbs / targetPathResDirRel};
  std::ofstream outResStream{resPathAbs};
  YAML::Emitter resEmitter{outResStream};
  resEmitter << resNode;

  if (!WriteMeta(resPathAbs, res.GetGuid(), NativeResourceImporter{})) {
    return false;
  }

  res.SetName(targetPathResDirRel.stem().string());

  mGuidToSrcAbsPath.insert_or_assign(res.GetGuid(), resPathAbs);
  mGuidToType.insert_or_assign(res.GetGuid(), rttr::type::get(res));
  mGuidToResAbsPath.insert_or_assign(res.GetGuid(), resPathAbs);
  mSrcAbsPathToGuid.insert_or_assign(resPathAbs, res.GetGuid());

  gResourceManager.Add(std::addressof(res));
  gResourceManager.UpdateMappings(CreateMappings());
  return true;
}


auto ResourceDB::SaveResource(NativeResource const& res) -> void {
  if (auto const it{mGuidToSrcAbsPath.find(res.GetGuid())}; it != std::end(mGuidToSrcAbsPath)) {
    std::ofstream outStream{it->second};
    YAML::Emitter emitter{outStream};
    emitter << res.Serialize();
  }
}


auto ResourceDB::ImportResource(std::filesystem::path const& resPathResDirRel, ObserverPtr<ResourceImporter> importer) -> bool {
  // Temporary to keep a potentially freshly created importer alive in this stack frame.
  // Use the observer ptr, do not access this directly.
  std::unique_ptr<ResourceImporter> ownedImporter;

  if (!importer) {
    // If we weren't passed an importer instance, we use a new default one.
    ownedImporter = GetNewImporterForResourceFile(resPathResDirRel);
    importer = ownedImporter.get();

    if (!importer) {
      return false;
    }
  }

  auto guid{Guid::Invalid()};

  // If a meta file already exists for the resource, we attempt to reimport it and keep its Guid.
  if (LoadMeta(GetResourceDirectoryAbsolutePath() / resPathResDirRel, std::addressof(guid), nullptr) && gResourceManager.IsLoaded(guid)) {
    if (*mSelectedObjectPtr == gResourceManager.GetOrLoad(guid)) {
      *mSelectedObjectPtr = nullptr;
    }

    gResourceManager.Unload(guid);
  }

  // If there is no meta file, we proceed with a regular import.
  if (!guid.IsValid()) {
    guid = Guid::Generate();
  }

  if (!InternalImportResource(resPathResDirRel, mGuidToSrcAbsPath, mGuidToResAbsPath, mSrcAbsPathToGuid, mGuidToType, *importer, guid)) {
    return false;
  }

  gResourceManager.UpdateMappings(CreateMappings());
  return true;
}


auto ResourceDB::MoveResource(Guid const& guid, std::filesystem::path const& targetPathResDirRel) -> bool {
  auto const it{mGuidToSrcAbsPath.find(guid)};

  if (it == std::end(mGuidToSrcAbsPath)) {
    return false;
  }

  auto const srcPathAbs{it->second};
  auto const srcMetaPathAbs{GetMetaPath(srcPathAbs)};
  auto const dstPathAbs{mResDirAbs / targetPathResDirRel};
  auto const dstMetaPathAbs{GetMetaPath(dstPathAbs)};

  if (!exists(srcPathAbs) || !exists(srcMetaPathAbs) || exists(dstPathAbs) || exists(dstMetaPathAbs)) {
    return false;
  }

  rename(srcPathAbs, dstPathAbs);
  rename(srcMetaPathAbs, dstMetaPathAbs);
  Refresh();

  return true;
}


auto ResourceDB::MoveDirectory(std::filesystem::path const& srcPathResDirRel, std::filesystem::path const& dstPathResDirRel) -> bool {
  auto const srcPathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / srcPathResDirRel)};
  auto const dstPathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / dstPathResDirRel)};

  if (!exists(srcPathAbs) || exists(dstPathAbs) || !is_directory(srcPathAbs) || equivalent(srcPathAbs, GetResourceDirectoryAbsolutePath())) {
    return false;
  }

  rename(srcPathAbs, dstPathAbs);
  Refresh();

  return true;
}


auto ResourceDB::DeleteResource(Guid const& guid) -> void {
  gResourceManager.Unload(guid);

  if (auto const it{mGuidToSrcAbsPath.find(guid)}; it != std::end(mGuidToSrcAbsPath)) {
    std::filesystem::remove(it->second);
    std::filesystem::remove(GetMetaPath(it->second));
    mSrcAbsPathToGuid.erase(it->second);
    mGuidToSrcAbsPath.erase(it);
  }

  mGuidToResAbsPath.erase(guid);
  mGuidToType.erase(guid);
  gResourceManager.UpdateMappings(CreateMappings());
}


auto ResourceDB::DeleteDirectory(std::filesystem::path const& pathResDirRel) -> bool {
  auto const pathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / pathResDirRel)};

  if (!exists(pathAbs) || !is_directory(pathAbs)) {
    return false;
  }

  std::vector<Guid> resourcesToDelete;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{pathAbs}) {
    if (auto const it{mSrcAbsPathToGuid.find(entry.path())}; it != std::end(mSrcAbsPathToGuid)) {
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
  return mGuidToSrcAbsPath.contains(res.GetGuid());
}


auto ResourceDB::PathToGuid(std::filesystem::path const& pathResDirRel) -> Guid {
  if (auto const it{mSrcAbsPathToGuid.find(GetResourceDirectoryAbsolutePath() / pathResDirRel)}; it != std::end(mSrcAbsPathToGuid)) {
    return it->second;
  }

  return Guid::Invalid();
}


auto ResourceDB::GuidToPath(Guid const& guid) -> std::filesystem::path {
  if (auto const it{mGuidToSrcAbsPath.find(guid)}; it != std::end(mGuidToSrcAbsPath)) {
    return it->second.lexically_relative(GetResourceDirectoryAbsolutePath());
  }

  return {};
}


auto ResourceDB::GenerateUniqueResourceDirectoryRelativePath(std::filesystem::path const& targetPathResDirRel) const -> std::filesystem::path {
  return GenerateUniquePath(mResDirAbs / targetPathResDirRel);
}


auto ResourceDB::GetMetaPath(std::filesystem::path const& path) -> std::filesystem::path {
  return std::filesystem::path{path} += RESOURCE_META_FILE_EXT;
}


auto ResourceDB::IsMetaFile(std::filesystem::path const& path) -> bool {
  return path.extension() == RESOURCE_META_FILE_EXT;
}


auto ResourceDB::LoadMeta(std::filesystem::path const& resPathAbs, ObserverPtr<Guid> const guid, ObserverPtr<std::unique_ptr<ResourceImporter>> const importer) noexcept -> bool {
  auto const metaPathAbs{GetMetaPath(resPathAbs)};

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


auto ResourceDB::WriteMeta(std::filesystem::path const& resPathAbs, Guid const& guid, ResourceImporter const& importer) noexcept -> bool {
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

  auto const metaPathAbs{GetMetaPath(resPathAbs)};

  std::ofstream outStream{metaPathAbs, std::ios::out | std::ios::trunc};

  if (!outStream.is_open()) {
    return false;
  }

  YAML::Emitter metaEmitter{outStream};
  metaEmitter << metaNode;
  return true;
}


auto ResourceDB::GetNewImporterForResourceFile(std::filesystem::path const& path) -> std::unique_ptr<ResourceImporter> {
  for (auto const& importerType : rttr::type::get<ResourceImporter>().get_derived_classes()) {
    auto importerVariant{importerType.create()};
    std::unique_ptr<ResourceImporter> importer{importerVariant.get_value<ResourceImporter*>()};

    std::pmr::vector<std::string> supportedExtensions{&GetTmpMemRes()};
    importer->GetSupportedFileExtensions(supportedExtensions);

    for (auto const& ext : supportedExtensions) {
      if (ext == path.extension()) {
        return importer;
      }
    }
  }

  return {};
}
}
