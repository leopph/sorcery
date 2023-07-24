#include "ResourceDB.hpp"

#include "NativeResourceImporter.hpp"
#include "ResourceManager.hpp"
#include "Reflection.hpp"

#include <fstream>
#include <ranges>


namespace sorcery::mage {
auto ResourceDB::Refresh() -> void {
  std::map<Guid, std::filesystem::path> newMapping;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{ mResDirAbs }) {
    if (entry.path().extension() == ResourceManager::RESOURCE_META_FILE_EXT) {
      auto const metaNode{ YAML::LoadFile(entry.path().string()) };
      auto const guid{ metaNode["guid"].as<Guid>() };
      auto const resPath{ std::filesystem::path{ entry.path() }.replace_extension() };

      newMapping.emplace(guid, resPath);

      if (auto const it{ mGuidToAbsPath.find(guid) }; it == std::end(mGuidToAbsPath)) {
        ImportResource(relative(resPath, mResDirAbs));
      }
    }
  }

  for (auto const& guid : mGuidToAbsPath | std::views::keys) {
    if (!newMapping.contains(guid)) {
      DeleteResource(guid);
    }
  }

  mGuidToAbsPath = std::move(newMapping);
  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::ChangeProjectDir(std::filesystem::path const& projDirAbs) -> void {
  mResDirAbs = projDirAbs / RESOURCE_DIR_PROJ_REL;
  mGuidToAbsPath.clear();

  for (auto const& entry : std::filesystem::recursive_directory_iterator{ mResDirAbs }) {
    if (entry.path().extension() == ResourceManager::RESOURCE_META_FILE_EXT) {
      auto const metaNode{ YAML::LoadFile(entry.path().string()) };
      mGuidToAbsPath.insert_or_assign(metaNode["guid"].as<Guid>(), std::filesystem::path{ entry.path() }.replace_extension());
    }
  }

  gResourceManager.UnloadAll();
  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::GetResourceDirectoryAbsolutePath() -> std::filesystem::path const& {
  return mResDirAbs;
}


auto ResourceDB::CreateResource(std::shared_ptr<NativeResource>&& res, std::filesystem::path const& targetPathResDirRel) -> void {
  if (!res) {
    return;
  }

  if (!res->GetGuid().IsValid()) {
    res->SetGuid(Guid::Generate());
  }

  YAML::Node importerNode;
  importerNode["type"] = rttr::type::get<NativeResourceImporter>().get_name().to_string();
  importerNode["properties"] = ReflectionSerializeToYAML(NativeResourceImporter{});

  YAML::Node metaNode;
  metaNode["guid"] = res->GetGuid();
  metaNode["importer"] = importerNode;

  auto const targetPathAbs{ mResDirAbs / targetPathResDirRel };
  auto const targetMetaPathAbs{ targetPathAbs / ResourceManager::RESOURCE_META_FILE_EXT };

  std::ofstream outMetaStream{ targetMetaPathAbs };
  YAML::Emitter metaEmitter{ outMetaStream };
  metaEmitter << metaNode;

  res->SetName(targetPathResDirRel.stem().string());

  mGuidToAbsPath.insert_or_assign(res->GetGuid(), targetPathAbs);

  gResourceManager.Add(std::move(res));
  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::ImportResource(std::filesystem::path const& targetPathResDirRel) -> void {
  auto const targetPathAbs{ mResDirAbs / targetPathResDirRel };

  for (auto const& importerType : rttr::type::get<ResourceImporter>().get_derived_classes()) {
    auto importerVariant{ importerType.create() };
    auto& importer{ importerVariant.get_value<ResourceImporter>() };

    static std::vector<std::string> supportedExtensions;
    supportedExtensions.clear();
    importer.GetSupportedFileExtensions(supportedExtensions);

    for (auto const& ext : supportedExtensions) {
      if (ext == targetPathAbs.extension()) {
        YAML::Node importerNode;
        importerNode["type"] = importerType.get_name().to_string();
        importerNode["properties"] = ReflectionSerializeToYAML(importerVariant);

        auto const guid{ Guid::Generate() };

        YAML::Node metaNode;
        metaNode["guid"] = guid;
        metaNode["importer"] = importerNode;

        std::ofstream outMetaStream{ targetPathAbs / ResourceManager::RESOURCE_META_FILE_EXT };
        YAML::Emitter metaEmitter{ outMetaStream };
        metaEmitter << metaNode;
        outMetaStream.flush();

        mGuidToAbsPath.insert_or_assign(guid, targetPathAbs);

        gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
        gResourceManager.LoadResource(guid);
        return;
      }
    }
  }
}


auto ResourceDB::MoveResource(Guid const& guid, std::filesystem::path const& targetPathResDirRel) -> void {
  auto const it{ mGuidToAbsPath.find(guid) };

  if (it == std::end(mGuidToAbsPath)) {
    return;
  }

  auto const srcPathAbs{ it->second };
  auto const srcMetaPathAbs{ srcPathAbs / ResourceManager::RESOURCE_META_FILE_EXT };
  auto const dstPathAbs{ mResDirAbs / targetPathResDirRel };
  auto const dstMetaPathAbs{ dstPathAbs / ResourceManager::RESOURCE_META_FILE_EXT };

  if (!exists(srcPathAbs) || !exists(srcMetaPathAbs) || exists(dstPathAbs) || exists(dstMetaPathAbs)) {
    return;
  }

  rename(srcPathAbs, dstPathAbs);
  rename(srcMetaPathAbs, dstMetaPathAbs);

  if (auto const rh{ gResourceManager.LoadResource(guid) }; rh.Get()) {
    rh.Get()->SetName(dstMetaPathAbs.stem().string());
  }

  gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
}


auto ResourceDB::DeleteResource(Guid const& guid) -> void {
  if (auto const it{ mGuidToAbsPath.find(guid) }; it != std::end(mGuidToAbsPath)) {
    gResourceManager.Unload(guid);
    mGuidToAbsPath.erase(it);
    gResourceManager.UpdateGuidPathMappings(mGuidToAbsPath);
  }
}


auto ResourceDB::GenerateUniqueResourceDirectoryRelativePath(std::filesystem::path const& targetPathResDirRel) const -> std::filesystem::path {
  return GenerateUniquePath(mResDirAbs / targetPathResDirRel);
}
}
