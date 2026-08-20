#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "Event.hpp"
#include "Guid.hpp"
#include "observer_ptr.hpp"
#include "resource_id.hpp"
#include "../ResourceImporters/resource_importer.hpp"


namespace sorcery::mage {
class EditorApp;
class ResourceDB;
class EditorDrawerRegistry;


class ProjectWindow {
public:
  explicit ProjectWindow(EditorApp& context, ResourceDB& resource_db, EditorDrawerRegistry& drawer_registry);
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


  struct ResourcePackageFileProjectItem {
    Guid guid;

    [[nodiscard]] friend
    auto operator==(
      ResourcePackageFileProjectItem const& lhs,
      ResourcePackageFileProjectItem const& rhs
    ) -> bool = default;
  };


  struct NativeResourceFileProjectItem {
    Guid guid;

    [[nodiscard]] friend
    auto operator==(
      NativeResourceFileProjectItem const& lhs,
      NativeResourceFileProjectItem const& rhs
    ) -> bool = default;
  };


  struct SubresourceProjectItem {
    ResourceId id;

    [[nodiscard]] friend
    auto operator==(
      SubresourceProjectItem const& lhs,
      SubresourceProjectItem const& rhs
    ) -> bool = default;
  };


  using ProjectItem = std::variant<
    DirectoryProjectItem,
    ResourcePackageFileProjectItem,
    NativeResourceFileProjectItem,
    SubresourceProjectItem
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


  struct FileImportContext {
    std::unique_ptr<ResourceImporter> importer;
    std::filesystem::path src_path_abs;
    std::filesystem::path dst_path_abs;
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
    kCopyResourceId,
    kUnloadResource,
    kUnloadAllResourcesInFile,

    kMoveToFolder
  };


  struct ProjectCommand {
    ProjectCommandKind kind;
    ProjectItem target;
  };


  struct ImportSettingsContext {
    Guid guid;
    std::filesystem::path src_path_res_dir_rel;
    std::unique_ptr<ResourceImporter> importer;
    bool dirty{false};
  };


  [[nodiscard]] static
  auto IsDirectory(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto IsResourcePackageFile(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto IsNativeResourceFile(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto IsSubresource(ProjectTreeNode const& node) -> bool;

  [[nodiscard]] static
  auto ShouldDrawAsTree(ProjectTreeNode const& node) -> bool;

  auto DrawNode(ProjectTreeNode const& node) -> void;

  auto SelectItem(ProjectItem const& item) -> void;
  auto ClearSelection() -> void;
  auto SetEditorSelectionTo(ProjectItem const& item) const -> void;
  auto ValidateSelection() -> void;

  auto DrawContextMenu(ProjectItem const& item) -> void;
  auto DrawDirectoryContextMenu(DirectoryProjectItem const& item) -> void;
  auto DrawNativeResourceFileContextMenu(NativeResourceFileProjectItem const& item) -> void;
  auto DrawResourcePackageFileContextMenu(ResourcePackageFileProjectItem const& item) -> void;
  auto DrawSubresourceContextMenu(SubresourceProjectItem const& item) -> void;

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
  auto CommitResourceFileRename(Guid const& guid, std::string_view new_name) -> bool;

  [[nodiscard]]
  auto IsResourceRootDirectory(DirectoryProjectItem const& item) const -> bool;

  [[nodiscard]]
  auto IsResourceRootDirectory(std::filesystem::path const& path_abs) const -> bool;

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
  auto ExecuteDeleteResourceFile(Guid const& target) -> void;
  auto ExecuteShowInExplorer(ProjectItem const& target) const -> void;
  auto ExecuteReimport(ProjectItem const& target) const -> void;
  auto OpenImportSettings(ProjectItem const& target) -> void;
  auto ExecuteCopyGuid(ProjectItem const& target) const -> void;
  auto ExecuteCopyResourceId(ProjectItem const& target) const -> void;
  auto ExecuteUnloadResource(ProjectItem const& target) -> void;
  auto ExecuteUnloadAllResourcesInFile(ProjectItem const& target) -> void;
  auto ExecuteUnloadResourceById(ResourceId const& target) -> void;
  auto ExecuteMoveToFolder(ProjectItem const& target) -> void;

  ObserverPtr<EditorApp> app_;
  ObserverPtr<ResourceDB> resource_db_;
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
  constexpr static std::string_view kResPackNodeDragDropTypeStr{"DRAG_DROP_RES_PACK_TYPE"};
};
}
