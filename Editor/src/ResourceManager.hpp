#pragma once

#include <unordered_map>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "Object.hpp"
#include "ObjectWrappers/ObjectWrapperManager.hpp"


namespace leopph::editor {
class ResourceManager {
  static inline std::filesystem::path META_FILE_EXT{ ".leopphasset" };

  std::vector<std::shared_ptr<Object>> mAssets;
  std::unordered_map<std::filesystem::path, ObserverPtr<Object>> mPathToAsset;
  std::unordered_map<ObserverPtr<Object const>, std::filesystem::path> mAssetToPath;

  std::vector<std::filesystem::path> mKnownAbsoluteResourcePaths;
  std::unordered_map<std::filesystem::path, std::shared_ptr<Object>> mLoadedResources;

  std::filesystem::path mRootDirAbs{ std::filesystem::absolute(".") };
  std::shared_ptr<ObjectWrapperManager> mWrapperManager;
  ObserverPtr<Context> mContext;

  [[nodiscard]] auto GenerateMetaForNewResource(std::filesystem::path const& absoluteResourcePath) const -> YAML::Node;

public:
  auto Refresh() -> void;

  auto CreateResource(std::shared_ptr<NativeAsset> obj, std::filesystem::path const& relativeResourcePath) -> void;
  auto MoveResource(std::filesystem::path const& relativeOldPath, std::filesystem::path const& relativeNewPath) -> void;
  auto ImportResource(std::filesystem::path const& src, std::filesystem::path const& relativeTargetDir) -> void;
  auto LoadResource(std::filesystem::path const& relativeResourcePath) -> std::weak_ptr<Object>;

  ResourceManager(std::shared_ptr<ObjectWrapperManager> wrapperManager, ObserverPtr<Context> context);

  auto RegisterAsset(std::shared_ptr<Object> asset, std::filesystem::path const& srcPath) -> void;
  auto UnregisterAsset(std::filesystem::path const& path) -> std::shared_ptr<Object>;

  [[nodiscard]] auto GetPathFor(Object const* asset) const -> std::filesystem::path const&;
  [[nodiscard]] auto TryGetPathFor(Object const* asset) const -> std::filesystem::path;
  [[nodiscard]] auto GetAssetAt(std::filesystem::path const& srcPath) const -> Object*;
  [[nodiscard]] auto TryGetAssetAt(std::filesystem::path const& srcPath) const -> Object*;

  auto Clear() -> void;


  [[nodiscard]] auto begin() const noexcept -> decltype(auto) {
    return std::begin(mPathToAsset);
  }


  [[nodiscard]] auto end() const noexcept -> decltype(auto) {
    return std::end(mPathToAsset);
  }


  [[nodiscard]] auto cbegin() const noexcept -> decltype(auto) {
    return std::cbegin(mPathToAsset);
  }


  [[nodiscard]] auto cend() const noexcept -> decltype(auto) {
    return std::cend(mPathToAsset);
  }
};
}
