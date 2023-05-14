#include "ResourceManager.hpp"

#include "Asset.hpp"
#include "Util.hpp"

#include <fstream>
#include <sstream>


namespace leopph::editor {
auto ResourceManager::GetAbsoluteMetaPathFromSource(std::filesystem::path const& src) -> std::filesystem::path {
  return absolute(src.parent_path() / src.filename() += META_FILE_EXT);
}


ResourceManager::ResourceManager(std::shared_ptr<ObjectWrapperManager> wrapperManager) :
  mWrapperManager{ std::move(wrapperManager) } {}


auto ResourceManager::ImportAsset(std::filesystem::path const& src, std::filesystem::path const& targetDir, std::filesystem::path const& cacheDir) -> std::weak_ptr<Object> {
  throw std::exception{ "Not implemented" };
}


auto ResourceManager::LoadAsset(std::filesystem::path const& src, std::filesystem::path const& cacheDir) -> std::weak_ptr<Object> {
  throw std::exception{ "Not implemented" };
}


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
