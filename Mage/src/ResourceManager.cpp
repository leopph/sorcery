#include "ResourceManager.hpp"

#include "Asset.hpp"
#include "Util.hpp"

#include <fstream>

#include "EditorContext.hpp"


namespace sorcery::mage {
auto ResourceManager::GenerateMetaForNewResource(std::filesystem::path const& absoluteResourcePath) const -> YAML::Node {
  static std::vector<ObserverPtr<ObjectWrapper>> wrappers;
  wrappers.clear();

  YAML::Node metaNode;
  bool foundLoader{ false };

  for (auto const& wrapper : mWrapperManager->GetWrappers(wrappers)) {
    for (auto const& ext : wrapper->GetLoader().GetSupportedExtensions()) {
      if (ToLower(absoluteResourcePath.extension().string()) == ToLower(ext)) {
        metaNode["guid"] = Guid::Generate().ToString();
        metaNode["type"] = static_cast<int>(wrapper->GetWrappedType());
        metaNode["importPrecedence"] = wrapper->GetLoader().GetPrecedence();
        foundLoader = true;
        break;
      }
    }

    if (foundLoader) {
      break;
    }
  }

  if (!foundLoader) {
    throw std::runtime_error{ std::format("Failed to find loader for resource file at {}.", absoluteResourcePath.string()) };
  }

  return metaNode;
}


auto ResourceManager::Refresh() -> void {
  decltype(mKnownAbsoluteResourcePaths) newResourcePaths;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{ mContext->GetAssetDirectoryAbsolute() }) {
    if (entry.is_directory()) {
      continue;
    }

    if (!entry.path().has_extension()) {
      throw std::runtime_error{ "Can't handle resource file with no extension." };
    }

    if (entry.path().extension() == META_FILE_EXT) {
      if (!exists(std::filesystem::path{ entry.path() }.replace_extension())) {
        remove(entry.path());
      }
      continue;
    }

    static std::vector<ObserverPtr<ObjectWrapper>> wrappers;
    wrappers.clear();

    auto const metaPath{ std::filesystem::path{ entry.path() } += META_FILE_EXT };

    if (!exists(metaPath)) {
      auto const metaNode{ GenerateMetaForNewResource(entry.path()) };

      std::ofstream metaOut{ metaPath };

      if (!metaOut.is_open()) {
        throw std::runtime_error{ std::format("Failed to create meta file at {}.", metaPath.string()) };
      }

      metaOut << Dump(metaNode);
    }

    newResourcePaths.emplace_back(absolute(entry.path()));
  }

  for (auto const& knownPath : mKnownAbsoluteResourcePaths) {
    if (mLoadedResources.contains(knownPath)) {
      bool stillNeeded{ false };

      for (auto const& newPath : newResourcePaths) {
        if (exists(knownPath) && equivalent(knownPath, newPath)) {
          stillNeeded = true;
          break;
        }
      }

      if (!stillNeeded) {
        mLoadedResources.erase(knownPath);
      }
    }
  }

  mKnownAbsoluteResourcePaths = std::move(newResourcePaths);
}


auto ResourceManager::CreateResource(std::shared_ptr<NativeAsset> obj, std::filesystem::path const& relativeResourcePath) -> void {
  if (!obj) {
    throw std::runtime_error{ "Cannot create resource from null object." };
  }

  auto const absoluteResourcePath{ mContext->GetAssetDirectoryAbsolute() / relativeResourcePath };

  std::vector<std::uint8_t> static bytes;
  bytes.clear();
  obj->Serialize(bytes);

  std::ofstream out{ absoluteResourcePath, std::ios::out | std::ios::binary };

  if (!out.is_open()) {
    throw std::runtime_error{ std::format("Failed to create resource at {}.", absoluteResourcePath.string()) };
  }

  std::ranges::copy(bytes, std::ostreambuf_iterator{ out });

  YAML::Node metaNode;
  metaNode["guid"] = obj->GetGuid().ToString();
  metaNode["type"] = static_cast<int>(obj->GetSerializationType());
  metaNode["importPrecedence"] = mWrapperManager->GetFor(obj->GetSerializationType()).GetLoader().GetPrecedence();

  auto const metaDst{ std::filesystem::path{ absoluteResourcePath } += META_FILE_EXT };
  std::ofstream metaOut{ metaDst };

  if (!metaOut.is_open()) {
    throw std::runtime_error{ std::format("Failed to create resource meta file at {}.", metaDst.string()) };
  }

  metaOut << Dump(metaNode);

  std::erase(mKnownAbsoluteResourcePaths, absoluteResourcePath);
  mKnownAbsoluteResourcePaths.emplace_back(absoluteResourcePath);

  mLoadedResources[absoluteResourcePath] = std::move(obj);
}


auto ResourceManager::MoveResource(std::filesystem::path const& relativeOldPath, std::filesystem::path const& relativeNewPath) -> void {
  auto const absoluteOldPath{ mContext->GetAssetDirectoryAbsolute() / relativeOldPath };
  auto const absoluteNewPath{ mContext->GetAssetDirectoryAbsolute() / relativeNewPath };

  if (!exists(absoluteOldPath)) {
    throw std::runtime_error{ "Can't move resource file because it does not exist." };
  }

  if (exists(absoluteNewPath)) {
    if (equivalent(absoluteOldPath, absoluteNewPath)) {
      return;
    }
    throw std::runtime_error{ "Can't move resource file because the destination path is already in use." };
  }

  rename(absoluteOldPath, absoluteNewPath);
  rename(std::filesystem::path{ absoluteOldPath } += META_FILE_EXT, std::filesystem::path{ absoluteNewPath } += META_FILE_EXT);

  for (auto& knownPath : mKnownAbsoluteResourcePaths) {
    if (equivalent(knownPath, absoluteOldPath)) {
      std::shared_ptr<Object> loadedResource;

      if (auto const it{ mLoadedResources.find(knownPath) }; it != std::end(mLoadedResources)) {
        loadedResource = std::move(it->second);
        mLoadedResources.erase(it);
      }

      knownPath = absoluteNewPath;

      if (loadedResource) {
        mLoadedResources[absoluteNewPath] = std::move(loadedResource);
      }

      return;
    }
  }

  throw std::runtime_error{ "Can't move resource file because it is not known to the Resource Manager." };
}


auto ResourceManager::ImportResource(std::filesystem::path const& src, std::filesystem::path const& relativeTargetDir) -> void {
  if (!exists(src)) {
    throw std::runtime_error{ "Can't import resource because the source file does not exist." };
  }

  auto const absoluteTargetDir{ mContext->GetAssetDirectoryAbsolute() / relativeTargetDir };
  auto const absoluteTargetPath{ GenerateUniquePath(absoluteTargetDir / src.filename()) };

  copy_file(src, absoluteTargetPath);

  mKnownAbsoluteResourcePaths.emplace_back(absoluteTargetPath);

  auto const metaNode{ GenerateMetaForNewResource(absoluteTargetPath) };

  auto const metaDst{ std::filesystem::path{ absoluteTargetPath } += META_FILE_EXT };
  std::ofstream metaOut{ metaDst };

  if (!metaOut.is_open()) {
    throw std::runtime_error{ std::format("Failed to create resource meta file at {}.", metaDst.string()) };
  }

  metaOut << Dump(metaNode);
}


auto ResourceManager::LoadResource(std::filesystem::path const& relativeResourcePath) -> std::weak_ptr<Object> {
  auto const absoluteResourcePath{ mContext->GetAssetDirectoryAbsolute() / relativeResourcePath };

  if (auto const it{ mLoadedResources.find(mContext->GetAssetDirectoryAbsolute() / relativeResourcePath) }; it != std::end(mLoadedResources)) {
    return it->second;
  }

  // TODO
  throw std::exception{ "Not implemented." };
}


ResourceManager::ResourceManager(std::shared_ptr<ObjectWrapperManager> wrapperManager, ObserverPtr<Context> const context) :
  mWrapperManager{ std::move(wrapperManager) },
  mContext{ context } {}


auto ResourceManager::RegisterAsset(std::shared_ptr<Object> asset, std::filesystem::path const& srcPath) -> void {
  mAssetToPath[asset.get()] = srcPath;
  mPathToAsset[srcPath] = asset.get();
  mAssets.emplace_back(std::move(asset));
}


auto ResourceManager::UnregisterAsset(std::filesystem::path const& path) -> std::shared_ptr<Object> {
  auto const asset{ mPathToAsset.at(path) };
  mPathToAsset.erase(path);
  mAssetToPath.erase(asset);

  std::shared_ptr<Object> ret;

  for (std::size_t i = 0; i < mAssets.size(); i++) {
    if (mAssets[i].get() == asset) {
      ret = std::move(mAssets[i]);
      mAssets.erase(std::begin(mAssets) + i);
    }
  }

  return ret;
}


auto ResourceManager::GetPathFor(Object const* asset) const -> std::filesystem::path const& {
  return mAssetToPath.at(asset);
}


auto ResourceManager::TryGetPathFor(Object const* asset) const -> std::filesystem::path {
  if (auto const it{ mAssetToPath.find(asset) }; it != std::end(mAssetToPath)) {
    return it->second;
  }
  return {};
}


auto ResourceManager::GetAssetAt(std::filesystem::path const& srcPath) const -> Object* {
  return mPathToAsset.at(srcPath);
}


auto ResourceManager::TryGetAssetAt(std::filesystem::path const& srcPath) const -> Object* {
  if (auto const it{ mPathToAsset.find(srcPath) }; it != std::end(mPathToAsset)) {
    return it->second;
  }
  return nullptr;
}


auto ResourceManager::Clear() -> void {
  mAssetToPath.clear();
  mPathToAsset.clear();
  mAssets.clear();
}
}
