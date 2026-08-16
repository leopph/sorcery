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
#include "ResourceImporters/resource_importer.hpp"


namespace sorcery::mage {
class EditorApp;
class ResourceDB;


class ProjectWindow {
public:
  explicit ProjectWindow(EditorApp& context, ResourceDB& resource_db);
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
    std::filesystem::path node_path_abs; // TODO remove later
    ProjectItem target;
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


  enum class ProjectCommandKind : std::uint8_t {
    kImportFilesIntoFolder,
    kCreateFolder,
    kBeginRename,
    kDelete,
    kShowInExplorer,

    kReimportFile,
    kOpenImportSettings,
    kCopyGuid,
    kCopyResourceId,
    kUnloadResource,
    kUnloadAllResourcesInFile,
    kOpenResource,
    kLocateParentFile
  };


  struct ProjectCommand {
    ProjectCommandKind kind;
    ProjectItem target;
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
  auto SetEditorSelectionTo(ProjectItem const& item) const -> void;
  auto ValidateSelection() -> void;

  auto DrawContextMenu(ProjectItem const& item) -> void;
  auto DrawDirectoryContextMenu(DirectoryProjectItem const& item) -> void;
  auto DrawNativeResourceFileContextMenu(NativeResourceFileProjectItem const& item) -> void;
  auto DrawResourcePackageFileContextMenu(ResourcePackageFileProjectItem const& item) -> void;
  auto DrawSubresourceContextMenu(SubresourceProjectItem const& item) -> void;

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

  auto ExecutePendingCommand() -> void;
  auto ExecuteImport(ProjectItem const& target) const -> void;
  auto CreateFolder(ProjectItem const& target) -> void;
  auto BeginRename(ProjectItem const& target) -> void;
  auto ExecuteDelete(ProjectItem const& target) -> void;
  auto ExecuteShowInExplorer(ProjectItem const& target) -> void;
  auto ExecuteReimport(ProjectItem const& target) -> void;
  auto OpenImportSettings(ProjectItem const& target) -> void;
  auto ExecuteCopyGuid(ProjectItem const& target) -> void;
  auto ExecuteCopyResourceId(ProjectItem const& target) -> void;
  auto ExecuteUnloadResource(ProjectItem const& target) -> void;
  auto ExecuteUnloadAllResourcesInFile(ProjectItem const& target) -> void;
  auto ExecuteOpenResource(ProjectItem const& target) -> void;
  auto ExecuteLocateParentFile(ProjectItem const& target) -> void;

  // Returns whether the drawn subtree was modified.
  [[nodiscard]]
  auto DrawFilesystemTree(
    std::filesystem::path const& node_path_abs,
    std::filesystem::path const& node_path_res_dir_rel,
    bool is_directory
  ) noexcept -> bool;

  auto DrawContextMenu() -> void;

  auto StartRenamingSelected() noexcept -> void;

  [[nodiscard]]
  auto TryImportFromSourceFile(
    ResourceImporter* importer,
    std::filesystem::path const& src_path_abs,
    std::filesystem::path const& dst_path_abs
  ) const -> bool;

  ObserverPtr<EditorApp> app_;
  ObserverPtr<ResourceDB> resource_db_;
  EventListenerHandle<> database_changed_listener_;

  std::optional<ProjectTreeNode> root_node_;
  std::optional<ProjectItem> selected_item_;
  std::optional<ProjectCommand> pending_command_;
  std::optional<ProjectItem> context_menu_target_;
  std::optional<RenameContext> rename_ctx_;
  std::optional<PendingRenameAction> pending_rename_action_;
  std::vector<FileImportContext> files_to_import_;
  bool open_import_modal_{false};
  bool open_context_menu_{false};
  bool should_rebuild_hierarchy_on_next_draw_{true};

  std::filesystem::path mSelectedPathResDirRel; // empty if not selected TODO remove

  constexpr static std::string_view kContextMenuId{"ContextMenu"};
  constexpr static std::string_view kDirNodeDragDropTypeStr{"NodeDragDropTypeStr"};
};
}
