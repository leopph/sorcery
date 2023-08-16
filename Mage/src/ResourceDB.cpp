#include "ResourceDB.hpp"

#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "ResourceManager.hpp"
#include "Util.hpp"
#include "ResourceImporters/NativeResourceImporter.hpp"

#include <fstream>
#include <ranges>


namespace sorcery::mage {
auto ResourceDB::InternalImportResource(std::filesystem::path const& resPathResDirRel, std::map<Guid, std::filesystem::path>& guidToSrcAbsPath, std::map<Guid, std::filesystem::path>& guidToResAbsPath, std::map<std::filesystem::path, Guid>& srcAbsPathToGuid, ResourceImporter& importer, Guid const& guid) const -> bool {
  YAML::Node importerNode;
  importerNode["type"] = rttr::type::get(importer).get_name().data();
  importerNode["properties"] = ReflectionSerializeToYaml(importer);

  YAML::Node metaNode;
  metaNode["guid"] = guid;
  metaNode["importer"] = importerNode;

  auto const resPathAbs{mResDirAbs / resPathResDirRel};
  auto const metaPathAbs{GetMetaPath(resPathAbs)};

  std::ofstream outMetaStream{metaPathAbs, std::ios::out | std::ios::trunc};
  YAML::Emitter metaEmitter{outMetaStream};
  metaEmitter << metaNode;

  guidToSrcAbsPath.insert_or_assign(guid, resPathAbs);
  srcAbsPathToGuid.insert_or_assign(resPathAbs, guid);

  std::vector<std::byte> resBytes;

  if (!importer.Import(resPathAbs, resBytes)) {
    return false;
  }

  if (!importer.IsNativeImporter()) {
    auto const resCacheFilePathAbs{mCacheDirAbs / static_cast<std::string>(guid) += ResourceManager::EXTERNAL_RESOURCE_EXT};
    std::ofstream outCache{resCacheFilePathAbs, std::ios::binary | std::ios::out};
    outCache.write(reinterpret_cast<char*>(resBytes.data()), std::ssize(resBytes));
    guidToResAbsPath.insert_or_assign(guid, resCacheFilePathAbs);
  } else {
    guidToResAbsPath.insert_or_assign(guid, resPathAbs);
  }

  return true;
}


ResourceDB::ResourceDB(ObserverPtr<Object>& selectedObjectPtr) :
  mSelectedObjectPtr{std::addressof(selectedObjectPtr)} {}


auto ResourceDB::Refresh() -> void {
  std::map<Guid, std::filesystem::path> newGuidToSrcAbsPath;
  std::map<Guid, std::filesystem::path> newGuidToResAbsPath;
  std::map<std::filesystem::path, Guid> newSrcAbsPathToGuid;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{mResDirAbs}) {
    if (IsMetaFile(entry.path())) {
      if (auto const resPathAbs{std::filesystem::path{entry.path()}.replace_extension()}; exists(resPathAbs)) {
        // If we find a resource-meta file pair, we take note of them

        auto const metaNode{YAML::LoadFile(entry.path().string())};
        auto const guid{metaNode["guid"].as<Guid>()};

        newGuidToSrcAbsPath.emplace(guid, resPathAbs);
        newSrcAbsPathToGuid.emplace(resPathAbs, guid);
      } else {
        // If it's an orphaned meta file, we remove it
        remove(entry.path());
      }
    } else if (!exists(GetMetaPath(entry.path()))) {
      // If we find a file that is not a meta file, we attempt to import it as a resource
      if (auto const importer{GetNewImporterForResourceFile(entry.path())}) {
        InternalImportResource(entry.path().lexically_relative(GetResourceDirectoryAbsolutePath()), newGuidToSrcAbsPath, newGuidToResAbsPath, newSrcAbsPathToGuid, *importer, Guid::Generate());
      }
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
    DeleteResource(guid);
  }

  // We rename loaded resources that have been moved in the file system
  for (auto const& [guid, pathAbs] : newGuidToSrcAbsPath) {
    if (auto const it{mGuidToSrcAbsPath.find(guid)}; it != std::end(mGuidToSrcAbsPath) && it->second != pathAbs && gResourceManager.IsLoaded(guid)) {
      gResourceManager.GetOrLoad(guid)->SetName(pathAbs.stem().string());
    }
  }

  mGuidToSrcAbsPath = std::move(newGuidToSrcAbsPath);
  mGuidToResAbsPath = std::move(newGuidToResAbsPath);
  mSrcAbsPathToGuid = std::move(newSrcAbsPathToGuid);
  gResourceManager.UpdateGuidPathMappings(mGuidToResAbsPath);
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
  mSrcAbsPathToGuid.clear();

  Refresh();
}


auto ResourceDB::GetResourceDirectoryAbsolutePath() -> std::filesystem::path const& {
  return mResDirAbs;
}


auto ResourceDB::CreateResource(NativeResource& res, std::filesystem::path const& targetPathResDirRel) -> void {
  if (!res.GetGuid().IsValid()) {
    res.SetGuid(Guid::Generate());
  }

  auto const resNode{res.Serialize()};
  auto const targetPathAbs{mResDirAbs / targetPathResDirRel};
  std::ofstream outResStream{targetPathAbs};
  YAML::Emitter resEmitter{outResStream};
  resEmitter << resNode;

  YAML::Node importerNode;
  importerNode["type"] = rttr::type::get<NativeResourceImporter>().get_name().to_string();
  importerNode["properties"] = ReflectionSerializeToYaml(NativeResourceImporter{});

  YAML::Node metaNode;
  metaNode["guid"] = res.GetGuid();
  metaNode["importer"] = importerNode;

  std::ofstream outMetaStream{GetMetaPath(targetPathAbs)};
  YAML::Emitter metaEmitter{outMetaStream};
  metaEmitter << metaNode;

  res.SetName(targetPathResDirRel.stem().string());

  mGuidToSrcAbsPath.insert_or_assign(res.GetGuid(), targetPathAbs);
  mSrcAbsPathToGuid.insert_or_assign(targetPathAbs, res.GetGuid());

  gResourceManager.Add(std::addressof(res));
  gResourceManager.UpdateGuidPathMappings(mGuidToSrcAbsPath);
}


auto ResourceDB::SaveResource(NativeResource const& res) -> void {
  if (auto const it{mGuidToSrcAbsPath.find(res.GetGuid())}; it != std::end(mGuidToSrcAbsPath)) {
    std::ofstream outStream{it->second};
    YAML::Emitter emitter{outStream};
    emitter << res.Serialize();
  }
}


auto ResourceDB::ImportResource(std::filesystem::path const& resPathResDirRel, ObserverPtr<ResourceImporter> importer) -> void {
  // Temporary to keep a potentially freshly created importer alive in this stack frame.
  // Use the observer ptr, do not access this directly.
  std::unique_ptr<ResourceImporter> ownedImporter;

  if (!importer) {
    // If we weren't passed an importer instance, we use a new default one.
    ownedImporter = GetNewImporterForResourceFile(resPathResDirRel);
    importer = ownedImporter.get();
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

  InternalImportResource(resPathResDirRel, mGuidToSrcAbsPath, mGuidToResAbsPath, mSrcAbsPathToGuid, *importer, guid);
  gResourceManager.UpdateGuidPathMappings(mGuidToSrcAbsPath);
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
  gResourceManager.UpdateGuidPathMappings(mGuidToSrcAbsPath);
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

  if (guid) {
    *guid = Guid::Parse(metaNode["guid"].as<std::string>());
  }

  if (importer) {
    auto const importerType{rttr::type::get_by_name(metaNode["importer"]["type"].as<std::string>())};
    assert(importerType.is_valid());

    auto importerVariant{importerType.create()};
    assert(importerVariant.is_valid());

    importer->reset(importerVariant.get_value<ResourceImporter*>());
    ReflectionDeserializeFromYaml(metaNode["importer"]["properties"], **importer);
  }

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
