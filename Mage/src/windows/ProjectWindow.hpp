#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "../AssetImporters/asset_importer.hpp"
#include "sorcery/event.hpp"
#include "sorcery/Guid.hpp"
#include "sorcery/observer_ptr.hpp"
#include "sorcery/asset_id.hpp"


namespace sorcery::mage {
class EditorApp;
class AssetDB;
class EditorDrawerRegistry;


class ProjectWindow {
public:
  explicit ProjectWindow(EditorApp& context, AssetDB& asset_db, EditorDrawerRegistry& drawer_registry);
  ProjectWindow(ProjectWindow const& other) = delete;
  ProjectWindow(ProjectWindow&& other) noexcept = delete;

  ~ProjectWindow();

  auto operator=(ProjectWindow const& other) -> ProjectWindow& = delete;
  auto operator=(ProjectWindow&& other) noexcept -> ProjectWindow& = delete;

  auto RebuildHierarchy() -> void;
  auto Draw() -> void;

private:
  struct DirectoryProjectItem {
    std::filesystem::path path_abs;

    [[nodiscard]] friend
    auto operator==(
      DirectoryProjectItem const& lhs,
      DirectoryProjectItem const& rhs
    ) -> bool = default;
  };


  struct AssetPackageFileProjectItem {
    Guid guid;

    [[nodiscard]] friend
    auto operator==(
      AssetPackageFileProjectItem const& lhs,
      AssetPackageFileProjectItem const& rhs
    ) -> bool = default;
  };


  struct NativeAssetFileProjectItem {
    Guid guid;

    [[nodiscard]] friend
    auto operator==(
      NativeAssetFileProjectItem const& lhs,
      NativeAssetFileProjectItem const& rhs
    ) -> bool = default;
  };


  struct SubassetProjectItem {
    AssetId id;

    [[nodiscard]] friend
    auto operator==(
      SubassetProjectItem const& lhs,
      SubassetProjectItem const& rhs
    ) -> bool = default;
  };


  using ProjectItem = std::variant<
    DirectoryProjectItem,
    AssetPackageFileProjectItem,
    NativeAssetFileProjectItem,
    SubassetProjectItem
  >;


  struct ProjectTreeNode {
    ProjectItem item;
    std::string display_name;
    std::string imgui_id;
    std::vector<ProjectTreeNode> children;
  };


  struct RenameContext {
    std::string new_name;
    std::string error_msg;
    ProjectItem target;
    bool focus_requested{true};
  };


  enum class PendingRenameAction : std::uint8_t {
    kCommit,
    kCancel
  };


  struct MoveToFolderContext {
    std::filesystem::path src_abs;
  };


  enum class ProjectCommandKind : std::uint8_t {
    kImportFilesIntoFolder,
    kCreateFolder,
    kCreateMaterial,
    kCreateScene,
    kBeginRename,
    kDelete,
    kShowInExplorer,

    kReimportFile,
    kOpenImportSettings,
    kCopyGuid,
    kCopyAssetId,
    kUnloadAsset,
    kUnloadAllAssetsInFile,

    kMoveToFolder
  };


  struct ProjectCommand {
    ProjectCommandKind kind;
    ProjectItem target;
  };


  struct ImportSettingsContext {
    std::filesystem::path src_path_asset_dir_rel;
    std::unique_ptr<AssetImporter> importer;
    bool dirty{false};
  };


  [[nodiscard]] static
  auto IsDirectory(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto IsAssetPackageFile(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto IsNativeAssetFile(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto IsSubasset(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto ShouldDrawAsTree(ProjectTreeNode const& node) -> bool;

  auto DrawNode(ProjectTreeNode const& node) -> void;

  auto SelectItem(ProjectItem const& item) -> void;
  auto ClearSelection() -> void;
  auto SetEditorSelectionTo(ProjectItem const& item) const -> void;
  auto ValidateSelection() -> void;

  auto DrawContextMenu(ProjectItem const& item) -> void;
  auto DrawDirectoryContextMenu(DirectoryProjectItem const& item) -> void;
  auto DrawNativeAssetFileContextMenu(NativeAssetFileProjectItem const& item) -> void;
  auto DrawAssetPackageFileContextMenu(AssetPackageFileProjectItem const& item) -> void;
  auto DrawSubassetContextMenu(SubassetProjectItem const& item) -> void;

  [[nodiscard]]
  auto CanDrag(ProjectItem const& item) const -> bool;

  [[nodiscard]] static
  auto CanDropOnto(ProjectItem const& item) -> bool;

  auto DrawDragSource(ProjectItem const& item) const -> void;
  auto DrawDropTarget(ProjectItem const& item) -> void;

  [[nodiscard]]
  auto CanRename(ProjectItem const& item) const -> bool;

  [[nodiscard]]
  auto IsRenaming(ProjectItem const& item) const -> bool;

  [[nodiscard]] static
  auto IsValidSingleFilename(std::string_view name) -> bool;

  auto DrawRenameInput() -> void;
  auto HandleRenameShortcut() -> void;
  auto ExecutePendingRenameAction() -> void;
  auto CommitRename() -> void;
  [[nodiscard]]
  auto CommitDirectoryRename(DirectoryProjectItem const& item, std::string_view new_name) -> bool;
  [[nodiscard]]
  auto CommitAssetFileRename(Guid const& guid, std::string_view new_name) -> bool;

  [[nodiscard]]
  auto IsAssetRootDirectory(DirectoryProjectItem const& item) const -> bool;

  [[nodiscard]]
  auto IsAssetRootDirectory(std::filesystem::path const& path_abs) const -> bool;

  [[nodiscard]]
  auto CanDelete(ProjectItem const& item) const -> bool;

  auto DrawImportSettingsDialog() -> void;

  auto ExecutePendingCommand() -> void;
  auto ExecuteImport(ProjectItem const& target) const -> void;
  auto ExecuteCreateFolder(ProjectItem const& target) const -> void;
  auto ExecuteCreateMaterial(ProjectItem const& target) -> void;
  auto ExecuteCreateScene(ProjectItem const& target) -> void;
  auto BeginRename(ProjectItem const& target) -> void;
  auto ExecuteDelete(ProjectItem const& target) -> void;
  auto ExecuteDeleteDirectory(DirectoryProjectItem const& target) -> void;
  auto ExecuteDeleteAssetFile(Guid const& target) -> void;
  auto ExecuteShowInExplorer(ProjectItem const& target) const -> void;
  auto ExecuteReimport(ProjectItem const& target) const -> void;
  auto OpenImportSettings(ProjectItem const& target) -> void;
  auto ExecuteCopyGuid(ProjectItem const& target) const -> void;
  auto ExecuteCopyAssetId(ProjectItem const& target) const -> void;
  auto ExecuteUnloadAsset(ProjectItem const& target) -> void;
  auto ExecuteUnloadAllAssetsInFile(ProjectItem const& target) -> void;
  auto ExecuteUnloadAssetById(AssetId const& target) -> void;
  auto ExecuteMoveToFolder(ProjectItem const& target) -> void;

  ObserverPtr<EditorApp> app_;
  ObserverPtr<AssetDB> asset_db_;
  ObserverPtr<EditorDrawerRegistry> drawer_registry_;
  EventListenerHandle<> database_changed_listener_;

  std::optional<ProjectTreeNode> root_node_;
  std::optional<ProjectItem> selected_item_;
  std::optional<ProjectCommand> pending_command_;
  std::optional<RenameContext> rename_ctx_;
  std::optional<PendingRenameAction> pending_rename_action_;
  std::optional<MoveToFolderContext> move_to_folder_ctx_;
  std::optional<ImportSettingsContext> import_settings_ctx_;
  bool open_import_settings_dialog_{false};
  bool should_rebuild_hierarchy_on_next_draw_{true};

  constexpr static std::string_view kContextMenuId{"ContextMenu"};
  constexpr static std::string_view kDirNodeDragDropTypeStr{"DRAG_DROP_DIR_TYPE"};
  constexpr static std::string_view kAssetPackNodeDragDropTypeStr{"DRAG_DROP_ASSET_PACK_TYPE"};
};
}
