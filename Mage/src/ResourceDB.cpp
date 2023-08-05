#include "ResourceDB.hpp"

#include "NativeResourceImporter.hpp"
#include "ResourceManager.hpp"
#include "Reflection.hpp"
#include "Util.hpp"

#include <fstream>
#include <ranges>


namespace sorcery::mage {
auto ResourceDB::InternalImportResource(std::filesystem::path const& targetPathResDirRel, std::map<Guid, std::filesystem::path>& guidToAbsPath, std::map<std::filesystem::path, Guid>& absPathToGuid) const -> void {
  auto const importerNode{FindImporterForResourceFile(targetPathResDirRel)};

  if (importerNode.IsNull()) {
    return;
  }

  auto const guid{Guid::Generate()};

  YAML::Node metaNode;
  metaNode["guid"] = guid;
  metaNode["importer"] = importerNode;

  auto const targetPathAbs{mResDirAbs / targetPathResDirRel};

  std::ofstream outMetaStream{ResourceManager::GetMetaPath(targetPathAbs)};
  YAML::Emitter metaEmitter{outMetaStream};
  metaEmitter << metaNode;
  outMetaStream.flush();

  guidToAbsPath.insert_or_assign(guid, targetPathAbs);
  absPathToGuid.insert_or_assign(targetPathAbs, guid);
}


auto ResourceDB::Refresh() -> void {
  std::map<Guid, std::filesystem::path> newGuidToAbsPath;
  std::map<std::filesystem::path, Guid> newAbsPathToGuid;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{mResDirAbs}) {
    if (ResourceManager::IsMetaFile(entry.path())) {
      auto const metaNode{YAML::LoadFile(entry.path().string())};
      auto const guid{metaNode["guid"].as<Guid>()};
      auto const resPathAbs{std::filesystem::path{entry.path()}.replace_extension()};

      newGuidToAbsPath.emplace(guid, resPathAbs);
      newAbsPathToGuid.emplace(resPathAbs, guid);
    } else if (!exists(ResourceManager::GetMetaPath(entry.path()))) {
      InternalImportResource(entry.path().lexically_relative(GetResourceDirectoryAbsolutePath()), newGuidToAbsPath, newAbsPathToGuid);
    }
  }

  std::vector<Guid> resourcesToDelete;

  for (auto const& guid : mGuidToAbsPath | std::views::keys) {
    if (!newGuidToAbsPath.contains(guid)) {
      // DeleteResource modifies this collection, so we collect the to be deleted resources first
      resourcesToDelete.emplace_back(guid);
    }
  }

  for (auto const& guid : resourcesToDelete) {
    DeleteResource(guid);
  }

  mGuidToAbsPath = std::move(newGuidToAbsPath);
  mAbsPathToGuid = std::move(newAbsPathToGuid);
  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::ChangeProjectDir(std::filesystem::path const& projDirAbs) -> void {
  mResDirAbs = projDirAbs / RESOURCE_DIR_PROJ_REL;

  if (!exists(mResDirAbs)) {
    create_directory(mResDirAbs);
  }

  for (auto const& guid : mGuidToAbsPath | std::views::keys) {
    gResourceManager.Unload(guid);
  }

  mGuidToAbsPath.clear();
  mAbsPathToGuid.clear();

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

  std::ofstream outMetaStream{ResourceManager::GetMetaPath(targetPathAbs)};
  YAML::Emitter metaEmitter{outMetaStream};
  metaEmitter << metaNode;

  res.SetName(targetPathResDirRel.stem().string());

  mGuidToAbsPath.insert_or_assign(res.GetGuid(), targetPathAbs);
  mAbsPathToGuid.insert_or_assign(targetPathAbs, res.GetGuid());

  gResourceManager.Add(std::addressof(res));
  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::SaveResource(NativeResource const& res) -> void {
  if (auto const it{mGuidToAbsPath.find(res.GetGuid())}; it != std::end(mGuidToAbsPath)) {
    std::ofstream outStream{it->second};
    YAML::Emitter emitter{outStream};
    emitter << res.Serialize();
  }
}


auto ResourceDB::ImportResource(std::filesystem::path const& targetPathResDirRel) -> void {
  InternalImportResource(targetPathResDirRel, mGuidToAbsPath, mAbsPathToGuid);
  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::MoveResource(Guid const& guid, std::filesystem::path const& targetPathResDirRel) -> void {
  auto const it{mGuidToAbsPath.find(guid)};

  if (it == std::end(mGuidToAbsPath)) {
    return;
  }

  auto const srcPathAbs{it->second};
  auto const srcMetaPathAbs{ResourceManager::GetMetaPath(srcPathAbs)};
  auto const dstPathAbs{mResDirAbs / targetPathResDirRel};
  auto const dstMetaPathAbs{ResourceManager::GetMetaPath(dstPathAbs)};

  if (!exists(srcPathAbs) || !exists(srcMetaPathAbs) || exists(dstPathAbs) || exists(dstMetaPathAbs)) {
    return;
  }

  rename(srcPathAbs, dstPathAbs);
  rename(srcMetaPathAbs, dstMetaPathAbs);

  if (auto const rh{gResourceManager.Load(guid)}) {
    rh->SetName(dstMetaPathAbs.stem().string());
  }

  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::DeleteResource(Guid const& guid) -> void {
  if (auto const it{mGuidToAbsPath.find(guid)}; it != std::end(mGuidToAbsPath)) {
    std::filesystem::remove(it->second);
    std::filesystem::remove(ResourceManager::GetMetaPath(it->second));
    gResourceManager.Unload(guid);
    mAbsPathToGuid.erase(it->second);
    mGuidToAbsPath.erase(it);
    gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
  }
}


auto ResourceDB::IsSavedResource(NativeResource const& res) const -> bool {
  return mGuidToAbsPath.contains(res.GetGuid());
}


auto ResourceDB::PathToGuid(std::filesystem::path const& pathResDirRel) -> Guid {
  if (auto const it{mAbsPathToGuid.find(GetResourceDirectoryAbsolutePath() / pathResDirRel)}; it != std::end(mAbsPathToGuid)) {
    return it->second;
  }

  return Guid::Invalid();
}


auto ResourceDB::GenerateUniqueResourceDirectoryRelativePath(std::filesystem::path const& targetPathResDirRel) const -> std::filesystem::path {
  return GenerateUniquePath(mResDirAbs / targetPathResDirRel);
}


auto ResourceDB::FindImporterForResourceFile(std::filesystem::path const& targetPathResDirRel) const -> YAML::Node {
  auto const targetPathAbs{mResDirAbs / targetPathResDirRel};

  for (auto const& importerType : rttr::type::get<ResourceImporter>().get_derived_classes()) {
    auto importerVariant{importerType.create()};
    std::unique_ptr<ResourceImporter> const importer{importerVariant.get_value<ResourceImporter*>()};

    static std::vector<std::string> supportedExtensions;
    supportedExtensions.clear();
    importer->GetSupportedFileExtensions(supportedExtensions);

    for (auto const& ext : supportedExtensions) {
      if (ext == targetPathAbs.extension()) {
        YAML::Node importerNode;
        importerNode["type"] = importerType.get_name().to_string();
        importerNode["properties"] = ReflectionSerializeToYaml(importerVariant);
        return importerNode;
      }
    }
  }

  return {};
}
}
