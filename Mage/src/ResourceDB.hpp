#pragma once

#include "Guid.hpp"
#include "NativeResource.hpp"
#include "observer_ptr.hpp"
#include "ResourceImporters/ResourceImporter.hpp"
#include "resource_manager.hpp"

#include <filesystem>
#include <map>


namespace sorcery::mage {
class ResourceDB {
public:
  explicit ResourceDB(Object*& selectedObjectPtr);

  /**
   * Refreshes the database by scanning the resource directory.
   * Registered files that cannot be found in the filesystem will be removed from the database.
   * Their associated objects will also be unloaded from memory.
   * All new files will be registered in the database.
   * All leftover meta files will be removed from the filesystem.
   */
  auto Refresh() -> void;


  /**
   * Changes the project directory to the given path.
   * If it does not exist, it will be created.
   * The database will be cleared and refreshed.
   */
  auto ChangeProjectDir(std::filesystem::path const& projDirAbs) -> void;


  /**
   * Returns the absolute path to the resource directory.
   */
  [[nodiscard]] auto GetResourceDirectoryAbsolutePath() -> std::filesystem::path const&;


  /**
   * Saves the native resource to the target resource file.
   * If the file already exists, it will be overwritten.
   * The database then takes ownership of the resource.
   * Returns an observer pointer to the resource.
   */
  auto CreateResource(std::unique_ptr<NativeResource>&& res,
                      std::filesystem::path const& target_path_res_dir_rel) -> ObserverPtr<NativeResource>;

  /**
   * Saves the native resource to the resource file it is already associated with.
   * If the resource is not associated with a file, it will not be saved.
   */
  auto SaveResource(NativeResource const& res) -> void;


  /**
   * Perform the import procedure for the resource file at the given path.
   * If the importer is not passed, a new one will be created based on the file extension.
   * Returns whether the import was successful.
   */
  [[nodiscard]] auto ImportResource(std::filesystem::path const& resPathResDirRel,
                                    ResourceImporter* importer = nullptr) -> bool;


  /**
   * Moves the resource file associated with the guid to the target path.
   * If the resource file does not exist, or the target path already exists, the move will fail.
   * Returns whether the move was successful.
   */
  [[nodiscard]] auto MoveResource(Guid const& guid, std::filesystem::path const& targetPathResDirRel) -> bool;


  /**
   * Moves the source directory to the destination directory.
   * If the source directory does not exist, or the destination directory already exists, the move will fail.
   * Returns whether the move was successful.
   */
  [[nodiscard]] auto MoveDirectory(std::filesystem::path const& srcPathResDirRel,
                                   std::filesystem::path const& dstPathResDirRel) -> bool;


  /**
   * Deletes the resource file associated with the guid.
   * If the resource file does not exist, or the guid is invalid, the deletion will fail.
   */
  auto DeleteResource(Guid const& guid) -> void;


  /**
   * Deletes the directory at the given path.
   * If the path is not a directory, or does not exist, the deletion will fail.
   * Returns whether the deletion was successful.
   */
  [[nodiscard]] auto DeleteDirectory(std::filesystem::path const& pathResDirRel) -> bool;


  /**
   * Checks if the resource is saved in the database.
   * Returns true if the resource is saved, false otherwise.
   */
  [[nodiscard]] auto IsSavedResource(NativeResource const& res) const -> bool;


  /**
   * Returns the Guid of the resource file associated with the given path.
   * If the resource file does not exist, or is not associated with a Guid, an invalid Guid will be returned.
   */
  [[nodiscard]] auto PathToGuid(std::filesystem::path const& pathResDirRel) -> Guid;


  /**
   * Returns the resource directory relative path associated with the given Guid.
   * If the Guid is not associated with a resource file, an empty path will be returned.
   */
  [[nodiscard]] auto GuidToPath(Guid const& guid) -> std::filesystem::path;


  /**
   * Returns an importer object for the resource file at the given path.
   * If the file does not exist, or it is not a resource file, or the importer cannot be retrieved, the function will return nullptr.
   */
  [[nodiscard]] static auto GetImporterForResourceFile(
    std::filesystem::path const& res_path_abs) noexcept -> std::unique_ptr<ResourceImporter>;


  /**
   * Creates a new importer object for the resource file at the given path.
   * If no suitable importer could be found, a nullptr will be returned.
   * The function will not validate the resource file's existence or contents.
   */
  [[nodiscard]] static auto CreateNewImporterForResourceFile(
    std::filesystem::path const& path) -> std::unique_ptr<ResourceImporter>;


  constexpr static std::string_view kResourceMetaFileExt{".mojo"};

private:
  constexpr static std::string_view kResourceDirProjRel{"Resources"};
  constexpr static std::string_view kCacheDirProjRel{"Cache"};


  /**
 * Returns the path where the meta file for the file at the given path would be stored if it were a resource file.
 * The function will not validate the given path, nor will it check if the file exists or if it is even a resource.
 */
  [[nodiscard]] static auto MakeMetaPath(std::filesystem::path const& path) -> std::filesystem::path;


  /**
   * Returns if the given path is a plausible path to a meta file, that is, if its format is that of a valid meta file path.
   * The function will not validate the file's existence or contents.
   */
  [[nodiscard]] static auto IsMetaFile(std::filesystem::path const& path) -> bool;


  /**
 * Reads the meta file of the resource at the given path.
 * Optional arguments may be passed to retrieve the contents of the meta file.
 * If the file failed to load for any reason, the optional arguments will not be modified.
 * Returns whether the meta file was read successfully.
 */
  [[nodiscard]] static auto ReadMeta(std::filesystem::path const& resPathAbs, Guid* guid,
                                     std::unique_ptr<ResourceImporter>* importer) noexcept -> bool;


  /**
   * Creates a meta file for the resource at the given path.
   * If the Guid is invalid or the importer cannot be recognized, the meta file will not be created.
   * If the meta file already exists, it will be overwritten.
   * The function will not validate the resource file's existence or contents.
   * Returns whether the meta file was written successfully.
   */
  [[nodiscard]] static auto WriteMeta(std::filesystem::path const& resPathAbs, Guid const& guid,
                                      ResourceImporter const& importer) noexcept -> bool;


  /**
   * Performs the import procedure for the resource file at the given path.
   * Uses the passed mappings to store the results and does not update the ResourceManager's mappings.
   * Returns whether the import was successful.
   */
  [[nodiscard]] auto InternalImportResource(std::filesystem::path const& res_path_abs,
                                            std::map<Guid, std::filesystem::path>& guidToSrcAbsPath,
                                            std::map<Guid, std::filesystem::path>& guidToResAbsPath,
                                            std::map<std::filesystem::path, Guid>& srcAbsPathToGuid,
                                            std::map<Guid, rttr::type>& guidToType, ResourceImporter& importer,
                                            Guid const& guid) const -> bool;


  /**
   * Creates the mappings for the resources in the database.
   */
  [[nodiscard]] auto CreateMappings() const noexcept ->
    std::pair<std::map<ResourceId, ResourceManager::ResourceDescription>, std::map<Guid, std::filesystem::path>>;


  /**
   * Returns an absolute path to a file that is appropriate to store an external resource with the given Guid.
   * The function will not validate the Guid nor the resource.
   */
  [[nodiscard]] auto MakeExternalResourceBinaryPathAbs(Guid const& guid) const noexcept -> std::filesystem::path;


  /**
   * Assembles the external resource binary file from the passed arguments and stores them in the appropriate file.
   * If the Guid is invalid, the function will fail.
   * If the file already exists, it will be overwritten.
   * Returns whether the file was written successfully.
   */
  [[nodiscard]] auto WriteExternalResourceBinary(Guid const& guid, ExternalResourceCategory categ,
                                                 std::span<std::byte const> resBytes) const noexcept -> bool;


  std::filesystem::path res_dir_abs_;
  std::filesystem::path cache_dir_abs_;

  std::map<Guid, rttr::type> guid_to_type_;

  // Maps resource Guids to the source file of the resource
  std::map<Guid, std::filesystem::path> guid_to_src_abs_path_;
  // Maps resource source files to the guid of the resource
  std::map<std::filesystem::path, Guid> src_abs_path_to_guid_;
  // Maps resource Guids to the loadable file of the resource
  std::map<Guid, std::filesystem::path> guid_to_load_abs_path_;
  Object** selected_object_ptr_;
};
}
