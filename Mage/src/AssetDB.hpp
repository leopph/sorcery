#pragma once

#include <filesystem>
#include <map>

#include "AssetImporters/asset_importer.hpp"
#include "sorcery/event.hpp"
#include "sorcery/Guid.hpp"
#include "sorcery/observer_ptr.hpp"
#include "sorcery/host/assets/asset_manager.hpp"
#include "sorcery/host/assets/asset_package.hpp"
#include "sorcery/assets/NativeAsset.hpp"


namespace sorcery::mage {
class AssetDB {
public:
  struct AssetFileInfo {
    Guid guid;
    std::filesystem::path src_path_asset_dir_rel;
    std::filesystem::path load_path_abs;
    int subasset_count;
    bool is_native_asset;
  };


  struct AssetInfo {
    AssetId id;
    rttr::type type;
    std::string name;
  };


  explicit AssetDB(ObjectPtr<Object>& selected_object_ptr);

  /**
   * Refreshes the database by scanning the asset directory.
   * Registered files that cannot be found in the filesystem will be removed from the database.
   * Their associated objects will also be unloaded from memory.
   * All new files will be registered in the database.
   * All leftover meta files will be removed from the filesystem.
   */
  auto Refresh(
  ) -> void;


  /**
   * Changes the project directory to the given path.
   * If it does not exist, it will be created.
   * The database will be cleared and refreshed.
   */
  auto ChangeProjectDir(
    std::filesystem::path const& proj_dir_abs
  ) -> void;


  /**
   * Returns the absolute path to the asset directory.
   */
  [[nodiscard]]
  auto GetAssetDirectoryAbsolutePath(
  ) const -> std::filesystem::path const&;


  /**
   * Saves the native asset to the target asset file.
   * If the file already exists, it will be overwritten.
   * The database then takes ownership of the asset.
   * Returns a reference to the asset.
   */
  auto SaveAssetToFile(
    std::unique_ptr<NativeAsset>&& asset,
    std::filesystem::path const& target_path_asset_dir_rel
  ) -> AssetRef<NativeAsset>;


  /**
   * Saves the native asset to the asset file it is already associated with.
   * If the asset is not associated with a file, it will not be saved.
   */
  auto SaveAssetToFile(
    NativeAsset const& asset
  ) -> void;


  /**
   * Perform the import procedure for the asset file at the given path.
   * If the importer is not passed, a new one will be created based on the file extension.
   * Returns whether the import was successful.
   */
  [[nodiscard]]
  auto ImportAssetFile(
    std::filesystem::path const& asset_path_asset_dir_rel,
    AssetImporter* importer = nullptr
  ) -> bool;


  /**
   * Moves the asset file associated with the guid to the target path.
   * If the asset file does not exist, or the target path already exists, the move will fail.
   * Returns whether the move was successful.
   */
  [[nodiscard]]
  auto MoveAssetFile(
    Guid const& guid,
    std::filesystem::path const& target_path_asset_dir_rel
  ) -> bool;


  /**
   * Moves the source directory to the destination directory.
   * If the source directory does not exist, or the destination directory already exists, the move will fail.
   * Returns whether the move was successful.
   */
  [[nodiscard]]
  auto MoveDirectory(
    std::filesystem::path const& src_path_asset_dir_rel,
    std::filesystem::path const& dst_path_asset_dir_rel
  ) -> bool;


  /**
   * Deletes the asset file associated with the guid.
   * If the asset file does not exist, or the guid is invalid, the deletion will fail.
   */
  auto DeleteAssetFile(
    Guid const& guid
  ) -> void;


  /**
   * Deletes the directory at the given path.
   * If the path is not a directory, or does not exist, the deletion will fail.
   * Returns whether the deletion was successful.
   */
  [[nodiscard]]
  auto DeleteDirectory(
    std::filesystem::path const& path_asset_dir_rel
  ) -> bool;


  /**
   * Checks if the asset is saved in the database.
   * Returns true if the asset is saved, false otherwise.
   */
  [[nodiscard]]
  auto IsSavedAsset(
    NativeAsset const& asset
  ) const -> bool;


  /**
   * Returns the Guid of the asset file associated with the given path.
   * If the asset file does not exist, or is not associated with a Guid, an invalid Guid will be returned.
   */
  [[nodiscard]]
  auto PathToGuid(
    std::filesystem::path const& path_asset_dir_rel
  ) const -> Guid;


  /**
   * Returns the asset directory relative path associated with the given Guid.
   * If the Guid is not associated with an asset file, an empty path will be returned.
   */
  [[nodiscard]]
  auto GuidToPath(
    Guid const& guid
  ) const -> std::filesystem::path;


  /**
   * Returns the asset file info associated with the given Guid.
   * If the guid is invalid or not associated with an asset file, nullptr will be returned.
   */
  [[nodiscard]]
  auto GetFileInfo(
    Guid const& guid
  ) const -> ObserverPtr<AssetFileInfo const>;


  /**
   * Returns the asset info associated with the given AssetId.
   * If the AssetId is invalid or not associated with an asset, nullptr will be returned.
   */
  [[nodiscard]]
  auto GetAssetInfo(
    AssetId const& id
  ) const -> ObserverPtr<AssetInfo const>;


  /**
   * Returns all assets that were imported from the asset file associated with the given Guid.
   * If the guid is invalid or not associated with an asset file, the output vector will be empty.
   */
  auto GetAssetsInFile(
    Guid const& guid,
    std::vector<ObserverPtr<AssetInfo const>>& out
  ) const -> void;


  /**
   * Returns whether the asset file associated with the given GUID can be edited.
   * Invalid or unknown GUIDs are considered editable.
   */
  [[nodiscard]]
  auto IsAssetFileEditable(
    Guid const& guid
  ) const -> bool;


  /**
   * Returns whether the asset associated with the given asset ID can be edited.
   * Invalid or unknown asset IDs are considered editable.
   */
  [[nodiscard]]
  auto IsAssetEditable(
    AssetId const& id
  ) const -> bool;


  /**
   * Returns an importer object for the asset file at the given path.
   * If the file does not exist, or it is not an asset file, or the importer cannot be retrieved, the function will return nullptr.
   */
  [[nodiscard]] static
  auto GetImporterForAssetFile(
    std::filesystem::path const& asset_path_abs
  ) noexcept -> std::unique_ptr<AssetImporter>;


  /**
   * Creates a new importer object for the asset file at the given path.
   * If no suitable importer could be found, a nullptr will be returned.
   * The function will not validate the asset file's existence or contents.
   */
  [[nodiscard]] static
  auto CreateNewImporterForAssetFile(
    std::filesystem::path const& path) -> std::unique_ptr<AssetImporter>;


  /**
   * Returns if the given path is a plausible path to a meta file, that is, if its format is that of a valid meta file path.
   * The function will not validate the file's existence or contents.
   */
  [[nodiscard]] static
  auto IsMetaFile(
    std::filesystem::path const& path
  ) -> bool;


  /**
   * Fires when the database has changed, for example when an asset file has been added, removed or modified.
   */
  GuardedEventReference<> OnDatabaseChanged;


  constexpr static std::string_view kAssetMetaFileExt{".mojo"};

private:
  /**
 * Returns the path where the meta file for the file at the given path would be stored if it were an asset file.
 * The function will not validate the given path, nor will it check if the file exists or if it is even an asset.
 */
  [[nodiscard]] static
  auto MakeMetaPath(
    std::filesystem::path const& path
  ) -> std::filesystem::path;


  /**
 * Reads the meta file of the asset at the given path.
 * Optional arguments may be passed to retrieve the contents of the meta file.
 * If the file failed to load for any reason, the optional arguments will not be modified.
 * Returns whether the meta file was read successfully.
 */
  [[nodiscard]] static
  auto ReadMeta(
    std::filesystem::path const& asset_path_abs,
    Guid* guid,
    std::unique_ptr<AssetImporter>* importer
  ) noexcept -> bool;


  /**
   * Creates a meta file for the asset at the given path.
   * If the Guid is invalid or the importer cannot be recognized, the meta file will not be created.
   * If the meta file already exists, it will be overwritten.
   * The function will not validate the asset file's existence or contents.
   * Returns whether the meta file was written successfully.
   */
  [[nodiscard]] static
  auto WriteMeta(
    std::filesystem::path const& asset_path_abs,
    Guid const& guid,
    AssetImporter const& importer
  ) noexcept -> bool;


  /**
   * Performs the import procedure for the asset file at the given path.
   * Uses the passed mappings to store the results and does not update the AssetManager's mappings.
   * Returns whether the import was successful.
   */
  [[nodiscard]]
  auto InternalImportAsset(
    std::filesystem::path const& asset_path_abs,
    std::map<std::filesystem::path, Guid>& guid_by_src_abs_path,
    std::map<Guid, AssetFileInfo>& asset_file_info_by_guid,
    std::map<AssetId, AssetInfo>& asset_info_by_id,
    AssetImporter& importer, Guid const& guid
  ) const -> bool;


  /**
   * Creates the mappings for the assets in the database.
   */
  [[nodiscard]]
  auto CreateMappings(
  ) const noexcept -> std::pair<std::map<AssetId, AssetManager::AssetDescription>, std::map<
                                  Guid, std::filesystem::path>>;


  /**
   * Returns an absolute path to a file that is appropriate to store an external asset with the given Guid.
   * The function will not validate the Guid nor the asset.
   */
  [[nodiscard]]
  auto MakeExternalAssetBinaryPathAbs(
    Guid const& guid
  ) const noexcept -> std::filesystem::path;


  /**
   * Assembles the external asset binary file from the passed arguments and stores them in the appropriate file.
   * If the Guid is invalid, the function will fail.
   * If the file already exists, it will be overwritten.
   * Returns whether the file was written successfully.
   */
  [[nodiscard]]
  auto WriteBinaryAssetPackage(
    Guid const& guid,
    std::span<AssetImportResult const> imports
  ) const noexcept -> bool;


  /**
   * Unloads all assets that were loaded from the asset file associated with the given Guid.
   */
  auto UnloadAssetsFromFile(
    Guid const& guid
  ) -> void;


  /**
   * Clear object selection if the selected object's guid matches the given guid.
   */
  auto ClearSelectionIfGuid(
    Guid const& guid
  ) const -> void;


  std::filesystem::path asset_dir_abs_;
  std::filesystem::path cache_dir_abs_;

  std::map<std::filesystem::path, Guid> guid_by_src_abs_path_;
  std::map<Guid, AssetFileInfo> asset_file_info_by_guid_;
  std::map<AssetId, AssetInfo> asset_info_by_id_;

  ObserverPtr<ObjectPtr<Object>> selected_object_ptr_;

  Event<> db_changed_;

  constexpr static std::string_view kAssetDirProjRel{"Assets"};
  constexpr static std::string_view kCacheDirProjRel{"Cache"};
};
}
