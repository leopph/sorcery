#pragma once

#include <unordered_map>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "Resource.hpp"
#include "ObjectWrappers/ObjectWrapperManager.hpp"
#include "Serialization.hpp"


namespace sorcery::mage {
class ResourceManager {
  static inline std::filesystem::path META_FILE_EXT{ ".leopphasset" };

  std::vector<std::shared_ptr<Resource>> mAssets;
  std::unordered_map<std::filesystem::path, ObserverPtr<Resource>> mPathToAsset;
  std::unordered_map<ObserverPtr<Resource const>, std::filesystem::path> mAssetToPath;

  std::vector<std::filesystem::path> mKnownAbsoluteResourcePaths;
  std::unordered_map<std::filesystem::path, std::shared_ptr<Resource>> mLoadedResources;

  std::filesystem::path mRootDirAbs{ std::filesystem::absolute(".") };
  std::shared_ptr<ObjectWrapperManager> mWrapperManager;
  ObserverPtr<Context> mContext;

  [[nodiscard]] auto GenerateMetaForNewResource(std::filesystem::path const& absoluteResourcePath) const -> YAML::Node;

public:
  auto Refresh() -> void;

  auto CreateResource(std::shared_ptr<NativeResource> obj, std::filesystem::path const& relativeResourcePath) -> void;
  auto MoveResource(std::filesystem::path const& relativeOldPath, std::filesystem::path const& relativeNewPath) -> void;
  auto ImportResource(std::filesystem::path const& src, std::filesystem::path const& relativeTargetDir) -> void;
  auto LoadResource(std::filesystem::path const& relativeResourcePath) -> std::weak_ptr<Resource>;

  ResourceManager(std::shared_ptr<ObjectWrapperManager> wrapperManager, ObserverPtr<Context> context);

  auto RegisterAsset(std::shared_ptr<Resource> asset, std::filesystem::path const& srcPath) -> void;
  auto UnregisterAsset(std::filesystem::path const& path) -> std::shared_ptr<Resource>;

  [[nodiscard]] auto GetPathFor(Resource const* asset) const -> std::filesystem::path const&;
  [[nodiscard]] auto TryGetPathFor(Resource const* asset) const -> std::filesystem::path;
  [[nodiscard]] auto GetAssetAt(std::filesystem::path const& srcPath) const -> ObserverPtr<Resource>;
  [[nodiscard]] auto TryGetAssetAt(std::filesystem::path const& srcPath) const -> ObserverPtr<Resource>;

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
