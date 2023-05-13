#pragma once

#include <unordered_map>
#include <filesystem>
#include <memory>

#include "Object.hpp"
#include "ObjectWrappers/ObjectWrapperManager.hpp"


namespace leopph::editor {
class ResourceManager {
  static inline std::filesystem::path META_FILE_EXT{ ".leopphasset" };

  std::vector<std::shared_ptr<Object>> mAssets;
  std::unordered_map<std::filesystem::path, ObserverPtr<Object>> mPathToAsset;
  std::unordered_map<ObserverPtr<Object const>, std::filesystem::path> mAssetToPath;

  std::filesystem::path mRootDirAbs{ std::filesystem::absolute(".") };
  std::shared_ptr<ObjectWrapperManager> mWrapperManager;


  [[nodiscard]] static auto GetAbsoluteMetaPathFromSource(std::filesystem::path const& src) -> std::filesystem::path;

public:
  explicit ResourceManager(std::shared_ptr<ObjectWrapperManager> wrapperManager);

  auto ImportAsset(std::filesystem::path const& src, std::filesystem::path const& targetDir, std::filesystem::path const& cacheDir) -> std::weak_ptr<Object>;
  [[nodiscard]] auto LoadAsset(std::filesystem::path const& src, std::filesystem::path const& cacheDir) -> std::weak_ptr<Object>;

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
