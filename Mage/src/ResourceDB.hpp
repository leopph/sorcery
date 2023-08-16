#pragma once

#include "Guid.hpp"
#include "NativeResource.hpp"
#include "ResourceImporters/ResourceImporter.hpp"

#include <filesystem>
#include <map>


namespace sorcery::mage {
class ResourceDB {
  std::filesystem::path mResDirAbs;
  std::filesystem::path mCacheDirAbs;
  std::map<Guid, std::filesystem::path> mGuidToSrcAbsPath; // Maps resource Guids to the source file of the resource
  std::map<Guid, std::filesystem::path> mGuidToResAbsPath; // Maps resource Guids to the loadable file of the resource
  std::map<std::filesystem::path, Guid> mSrcAbsPathToGuid; // Maps resource source files to the guid of the resource
  ObserverPtr<ObserverPtr<Object>> mSelectedObjectPtr;

public:
  constexpr static std::string_view RESOURCE_META_FILE_EXT{".mojo"};

private:
  constexpr static std::string_view RESOURCE_DIR_PROJ_REL{"Resources"};
  constexpr static std::string_view CACHE_DIR_PROJ_REL{"Cache"};

  // Uses the passed mappings to store the results and does not update the ResourceManager's mappings.
  // Writes the passed importer into the target meta file.
  // Assigns the passed Guid to the resource.
  [[nodiscard]] auto InternalImportResource(std::filesystem::path const& resPathResDirRel, std::map<Guid, std::filesystem::path>& guidToSrcAbsPath, std::map<Guid, std::filesystem::path>& guidToResAbsPath, std::map<std::filesystem::path, Guid>& srcAbsPathToGuid, ResourceImporter& importer, Guid const& guid) const -> bool;

public:
  explicit ResourceDB(ObserverPtr<Object>& selectedObjectPtr);

  auto Refresh() -> void;
  auto ChangeProjectDir(std::filesystem::path const& projDirAbs) -> void;
  [[nodiscard]] auto GetResourceDirectoryAbsolutePath() -> std::filesystem::path const&;

  auto CreateResource(NativeResource& res, std::filesystem::path const& targetPathResDirRel) -> void;
  auto SaveResource(NativeResource const& res) -> void;
  auto ImportResource(std::filesystem::path const& resPathResDirRel, ObserverPtr<ResourceImporter> importer = nullptr) -> void;
  // Returns whether the move was successful.
  [[nodiscard]] auto MoveResource(Guid const& guid, std::filesystem::path const& targetPathResDirRel) -> bool;
  // Returns whether the move was successful.
  [[nodiscard]] auto MoveDirectory(std::filesystem::path const& srcPathResDirRel, std::filesystem::path const& dstPathResDirRel) -> bool;
  auto DeleteResource(Guid const& guid) -> void;
  [[nodiscard]] auto DeleteDirectory(std::filesystem::path const& pathResDirRel) -> bool;
  [[nodiscard]] auto IsSavedResource(NativeResource const& res) const -> bool;
  [[nodiscard]] auto PathToGuid(std::filesystem::path const& pathResDirRel) -> Guid;
  // Returns a resource directory relative path, or empty if not found.
  [[nodiscard]] auto GuidToPath(Guid const& guid) -> std::filesystem::path;

  [[nodiscard]] auto GenerateUniqueResourceDirectoryRelativePath(std::filesystem::path const& targetPathResDirRel) const -> std::filesystem::path;

  [[nodiscard]] static auto GetMetaPath(std::filesystem::path const& path) -> std::filesystem::path;
  [[nodiscard]] static auto IsMetaFile(std::filesystem::path const& path) -> bool;
  // If the meta file successfully loads, guid and importer will be set to the read values.
  // Nullptrs can be passed to skip loading certain pieces of information.
  // The arguments won't be changed if the meta file failes to load.
  [[nodiscard]] static auto LoadMeta(std::filesystem::path const& resPathAbs, ObserverPtr<Guid> guid, ObserverPtr<std::unique_ptr<ResourceImporter>> importer) noexcept -> bool;
  [[nodiscard]] static auto GetNewImporterForResourceFile(std::filesystem::path const& path) -> std::unique_ptr<ResourceImporter>;
};
}
