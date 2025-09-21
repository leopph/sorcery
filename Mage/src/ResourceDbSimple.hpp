#pragma once

#include <filesystem>
#include <string_view>
#include <map>

#include "Guid.hpp"
#include "NativeResource.hpp"
#include "Object.hpp"
#include "observer_ptr.hpp"
#include "ResourceImporters/ResourceImporter.hpp"


namespace sorcery {
class ResourceDbSimple {
public:
  explicit ResourceDbSimple(Object*& selected_obj_ptr);


  /**
   * Refreshes the database by scanning the resource directory.
   * Registered files that cannot be found in the filesystem will be removed from the database.
   * Their associated objects will also be unloaded from memory.
   * All new files will be registered in the database.
   */
  auto Refresh() -> void;


  /**
   * Changes the project directory to the given path.
   * If it does not exist, it will be created.
   * The database will be cleared and refreshed.
   */
  auto ChangeProjectDir(std::filesystem::path const& proj_dir_abs) -> void;


  /**
   * Returns the absolute path to the resource directory.
   */
  [[nodiscard]]
  auto GetResourceDirectoryAbsolutePath() -> std::filesystem::path const&;


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
  [[nodiscard]]
  auto ImportResource(std::filesystem::path const& res_path_res_dir_rel, ResourceImporter* importer = nullptr) -> bool;


  /**
   * Moves the resource file associated with the guid to the target path.
   * If the resource file does not exist, or the target path already exists, the move will fail.
   * Returns whether the move was successful.
   */
  [[nodiscard]]
  auto MoveResource(Guid const& guid, std::filesystem::path const& target_path_res_dir_rel) -> bool;


  /**
   * Moves the source directory to the destination directory.
   * If the source directory does not exist, or the destination directory already exists, the move will fail.
   * Returns whether the move was successful.
   */
  [[nodiscard]]
  auto MoveDirectory(std::filesystem::path const& src_path_res_dir_rel,
                     std::filesystem::path const& dst_path_res_dir_rel) -> bool;


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
  [[nodiscard]]
  auto DeleteDirectory(std::filesystem::path const& path_res_dir_rel) -> bool;


  /**
   * Checks if the resource is saved in the database.
   * Returns true if the resource is saved, false otherwise.
   */
  [[nodiscard]]
  auto IsSavedResource(NativeResource const& res) const -> bool;


  /**
   * Returns the Guid of the resource file associated with the given path.
   * If the resource file does not exist, or is not associated with a Guid, an invalid Guid will be returned.
   */
  [[nodiscard]]
  auto PathToGuid(std::filesystem::path const& path_res_dir_rel) -> Guid;


  /**
   * Returns the resource directory relative path associated with the given Guid.
   * If the Guid is not associated with a resource file, an empty path will be returned.
   */
  [[nodiscard]]
  auto GuidToPath(Guid const& guid) -> std::filesystem::path;

private:
  Object** selected_obj_ptr_;
  std::filesystem::path res_dir_path_abs_;
  std::map<Guid, std::filesystem::path> guid_to_path_abs_;
  std::map<std::filesystem::path, Guid> path_abs_to_guid_;

  constexpr static std::string_view kResourceDirProjRel{"Resources"};
};
}
