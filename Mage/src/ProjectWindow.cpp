#include "ProjectWindow.hpp"

#include <optional>
#include <queue>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <nfd.hpp>
#include <spdlog/spdlog.h>

#include "EditorApp.hpp"
#include "gui_helpers.hpp"
#include "Material.hpp"
#include "Platform.hpp"
#include "ReflectionDisplayProperties.hpp"
#include "util.hpp"


namespace sorcery::mage {
namespace {
template<typename T>
[[nodiscard]]
auto ExtractPayloadData(ImGuiPayload const& payload) -> ObserverPtr<T const> {
  if (!payload.Data || payload.DataSize != sizeof(T)) {
    return nullptr;
  }

  return ObserverPtr<T const>{static_cast<T const*>(payload.Data)};
}
}


ProjectWindow::ProjectWindow(EditorApp& context, ResourceDB& resource_db) :
  app_{&context},
  resource_db_{&resource_db},
  database_changed_listener_{
    resource_db.OnDatabaseChanged.add_listener([this] { should_rebuild_hierarchy_on_next_draw_ = true; })
  } {}


ProjectWindow::~ProjectWindow() {
  resource_db_->OnDatabaseChanged.remove_listener(database_changed_listener_);
}


auto ProjectWindow::RebuildHierarchy() -> void {
  spdlog::debug("Starting project window hierarchy rebuild.");

  root_node_ = ProjectTreeNode{
    .item = DirectoryProjectItem{.path_abs = resource_db_->GetResourceDirectoryAbsolutePath()},
    .display_name = resource_db_->GetResourceDirectoryAbsolutePath().filename().string(),
    .imgui_id = std::format("##{}", resource_db_->GetResourceDirectoryAbsolutePath().string()),
    .children = {}
  };

  std::queue<ProjectTreeNode*> nodes_to_process;
  nodes_to_process.emplace(&*root_node_);

  while (!nodes_to_process.empty()) {
    auto* const current_node{nodes_to_process.front()};
    nodes_to_process.pop();

    if (!IsDirectory(*current_node)) {
      continue;
    }

    auto& dir_item{std::get<DirectoryProjectItem>(current_node->item)};

    for (auto const& entry : std::filesystem::directory_iterator{dir_item.path_abs}) {
      if (entry.is_directory()) {
        current_node->children.emplace_back(ProjectTreeNode{
          .item = DirectoryProjectItem{entry.path()},
          .display_name = entry.path().filename().string(),
          .imgui_id = std::format("##{}", entry.path().string()),
          .children = {}
        });
      } else if (entry.is_regular_file()) {
        if (ResourceDB::IsMetaFile(entry.path())) {
          continue;
        }

        auto const res_dir_rel{relative(entry.path(), resource_db_->GetResourceDirectoryAbsolutePath())};
        auto const guid{resource_db_->PathToGuid(res_dir_rel)};

        if (!guid.IsValid()) {
          continue;
        }

        auto const file_info{resource_db_->GetFileInfo(guid)};

        if (!file_info) {
          continue;
        }

        if (file_info->is_native_resource) {
          current_node->children.emplace_back(ProjectTreeNode{
            .item = NativeResourceFileProjectItem{.guid = guid},
            .display_name = entry.path().filename().string(),
            .imgui_id = std::format("##{}", entry.path().string()),
            .children = {}
          });
        } else {
          std::vector<ObserverPtr<ResourceDB::ResourceInfo const>> subresources;
          resource_db_->GetResourcesInFile(guid, subresources);

          std::vector<ProjectTreeNode> subresource_nodes;

          for (auto const& subresource : subresources) {
            subresource_nodes.emplace_back(ProjectTreeNode{
              .item = SubresourceProjectItem{.id = subresource->id},
              .display_name = subresource->name,
              .imgui_id = std::format("##{}", std::string{subresource->id}),
              .children = {}
            });
          }

          current_node->children.emplace_back(ProjectTreeNode{
            .item = ResourcePackageFileProjectItem{.guid = guid},
            .display_name = entry.path().filename().string(),
            .imgui_id = std::format("##{}", entry.path().string()),
            .children = std::move(subresource_nodes)
          });
        }
      }
    }

    // Sort children by name.
    // Folders first, then files.
    // Subresources are not sorted.
    if (!std::holds_alternative<ResourcePackageFileProjectItem>(current_node->item)) {
      auto const not_dir_range{
        std::ranges::partition(current_node->children, [](ProjectTreeNode const& node) {
          return IsDirectory(node);
        })
      };

      std::ranges::sort(std::ranges::begin(current_node->children), std::ranges::begin(not_dir_range), {},
        &ProjectTreeNode::display_name);

      std::ranges::sort(not_dir_range, {}, &ProjectTreeNode::display_name);
    }

    for (auto& child_node : current_node->children) {
      nodes_to_process.emplace(&child_node);
    }
  }

  spdlog::debug("Finished project window hierarchy rebuild.");
}


auto ProjectWindow::Draw() -> void {
  if (should_rebuild_hierarchy_on_next_draw_) {
    RebuildHierarchy();
    ValidateSelection();
    should_rebuild_hierarchy_on_next_draw_ = false;
  }

  ImGui::SetNextWindowSizeConstraints(ImVec2{150, 150}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoCollapse)) {
    if (root_node_) {
      DrawNode(*root_node_);
    }
    /*if ((!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows) &&
         ImGui::IsAnyItemHovered() && (ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
                                       ImGui::IsMouseReleased(ImGuiMouseButton_Right))) || (
          !mSelectedPathResDirRel.empty() && !exists(
            app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel))) {
      mSelectedPathResDirRel.clear();
    }

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        open_context_menu_ = true;
      }
    }

    std::ignore = DrawFilesystemTree(app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath(), "", true);

    if (open_context_menu_) {
      open_context_menu_ = false;
      ImGui::OpenPopup(kContextMenuId.data());
    }

    try {
      DrawContextMenu();
    } catch ([[maybe_unused]] std::runtime_error const& ex) {
      ImGui::End();
      throw;
    }

    auto constexpr importModalId{"Import Settings"};

    if (open_import_modal_) {
      open_import_modal_ = false;
      ImGui::OpenPopup(importModalId);
    }

    if (ImGui::BeginPopupModal(importModalId)) {
      for (auto& [importer, srcPathAbs, dstPathAbs] : files_to_import_) {
        ImGui::SeparatorText(srcPathAbs.stem().string().c_str());
        ImGui::PushID(srcPathAbs.string().c_str());
        ReflectionDisplayProperties(*importer);
        ImGui::PopID();
      }

      if (ImGui::Button("Cancel")) {
        files_to_import_.clear();
        ImGui::CloseCurrentPopup();
      }

      ImGui::SameLine();

      if (ImGui::Button("Import")) {
        for (auto const& [importer, src_path_abs, dst_path_abs] : files_to_import_) {
          if (!TryImportFromSourceFile(importer.get(), src_path_abs, dst_path_abs)) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::End();
            throw std::runtime_error{std::format("Failed to import {}.", dst_path_abs.string())};
          }
        }
        files_to_import_.clear();
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }*/
  }
  ImGui::End();

  HandleRenameShortcut();
  ExecutePendingCommand();
  ExecutePendingRenameAction();
}


auto ProjectWindow::IsDirectory(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<DirectoryProjectItem>(node.item);
}


auto ProjectWindow::IsResourcePackageFile(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<ResourcePackageFileProjectItem>(node.item);
}


auto ProjectWindow::IsNativeResourceFile(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<NativeResourceFileProjectItem>(node.item);
}


auto ProjectWindow::IsSubresource(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<SubresourceProjectItem>(node.item);
}


auto ProjectWindow::ShouldDrawAsTree(ProjectTreeNode const& node) -> bool {
  return IsDirectory(node) || IsResourcePackageFile(node);
}


auto ProjectWindow::DrawNode(ProjectTreeNode const& node) -> void {
  auto const draw_as_tree{ShouldDrawAsTree(node)};
  auto const is_renaming{IsRenaming(node.item)};

  ImGuiTreeNodeFlags flags =
    ImGuiTreeNodeFlags_DefaultOpen |
    ImGuiTreeNodeFlags_OpenOnArrow |
    ImGuiTreeNodeFlags_SpanAvailWidth;

  if (!draw_as_tree) {
    flags |= ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen;
  }

  if (selected_item_ && *selected_item_ == node.item) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  if (is_renaming) {
    flags |= ImGuiTreeNodeFlags_AllowOverlap;
    ImGui::SetNextItemAllowOverlap();
  }

  auto const is_open{
    ImGui::TreeNodeEx(
      node.imgui_id.c_str(),
      flags,
      "%s",
      is_renaming ? "" : node.display_name.c_str()
    )
  };

  if (!is_renaming) {
    if (ImGui::IsItemHovered() &&
        (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right))) {
      SelectItem(node.item);
    }
  }

  if (is_renaming) {
    DrawRenameInput();
  } else {
    if (CanDrag(node.item)) {
      DrawDragSource(node.item);
    }

    if (CanDropOnto(node.item)) {
      DrawDropTarget(node.item);
    }

    DrawContextMenu(node.item);
  }

  if (draw_as_tree && is_open) {
    for (auto const& child_node : node.children) {
      DrawNode(child_node);
    }

    ImGui::TreePop();
  }
}


auto ProjectWindow::SelectItem(ProjectItem const& item) -> void {
  selected_item_ = item;
  SetEditorSelectionTo(item);
}


auto ProjectWindow::ClearSelection() -> void {
  selected_item_.reset();
  app_->SetSelectedObject(nullptr);
}


auto ProjectWindow::SetEditorSelectionTo(ProjectItem const& item) const -> void {
  std::visit(Overloaded{
    [this](DirectoryProjectItem const&) {
      app_->SetSelectedObject(nullptr);
    },

    [this](NativeResourceFileProjectItem const& file) {
      app_->SetSelectedObject(App::Instance().GetResourceManager().GetOrLoad(ResourceId{file.guid, 0}));
    },

    [this](ResourcePackageFileProjectItem const&) {
      app_->SetSelectedObject(nullptr);
    },

    [this](SubresourceProjectItem const& resource) {
      app_->SetSelectedObject(App::Instance().GetResourceManager().GetOrLoad(resource.id));
    }
  }, item);
}


auto ProjectWindow::ValidateSelection() -> void {
  if (!selected_item_) {
    return;
  }

  std::visit(Overloaded{
    [&](DirectoryProjectItem const& item) {
      if (!std::filesystem::is_directory(item.path_abs) || relative(item.path_abs,
            resource_db_->GetResourceDirectoryAbsolutePath()).empty()) {
        ClearSelection();
      }
    },
    [&](NativeResourceFileProjectItem const& item) {
      if (!resource_db_->GetFileInfo(item.guid)) {
        ClearSelection();
      }
    },
    [&](ResourcePackageFileProjectItem const& item) {
      if (!resource_db_->GetFileInfo(item.guid)) {
        ClearSelection();
      }
    },
    [&](SubresourceProjectItem const& item) {
      if (!resource_db_->GetResourceInfo(item.id)) {
        if (resource_db_->GetFileInfo(item.id.GetGuid())) {
          selected_item_ = ResourcePackageFileProjectItem{item.id.GetGuid()};
        } else {
          ClearSelection();
        }
      }
    }
  }, *selected_item_);
}


auto ProjectWindow::DrawContextMenu(ProjectItem const& item) -> void {
  if (!ImGui::BeginPopupContextItem()) {
    return;
  }

  std::visit(Overloaded{
    [this](DirectoryProjectItem const& folder) {
      DrawDirectoryContextMenu(folder);
    },
    [this](NativeResourceFileProjectItem const& file) {
      DrawNativeResourceFileContextMenu(file);
    },
    [this](ResourcePackageFileProjectItem const& file) {
      DrawResourcePackageFileContextMenu(file);
    },
    [this](SubresourceProjectItem const& resource) {
      DrawSubresourceContextMenu(resource);
    }
  }, item);

  ImGui::EndPopup();
}


auto ProjectWindow::DrawDirectoryContextMenu(DirectoryProjectItem const& item) -> void {
  if (ImGui::BeginMenu("New")) {
    if (ImGui::MenuItem("Folder")) {
      pending_command_ = ProjectCommand{
        .kind = ProjectCommandKind::kCreateFolder,
        .target = item
      };
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Material")) {
      pending_command_ = ProjectCommand{
        .kind = ProjectCommandKind::kCreateMaterial,
        .target = item
      };
    }

    if (ImGui::MenuItem("Scene")) {
      pending_command_ = ProjectCommand{
        .kind = ProjectCommandKind::kCreateScene,
        .target = item
      };
    }

    ImGui::EndMenu();
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Import New Resource")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kImportFilesIntoFolder,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Rename", nullptr, false, CanRename(item))) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kBeginRename,
      .target = item
    };
  }

  if (ImGui::MenuItem("Delete", nullptr, false, CanDelete(item))) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kDelete,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Show in Explorer")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kShowInExplorer,
      .target = item
    };
  }
}


auto ProjectWindow::DrawNativeResourceFileContextMenu(NativeResourceFileProjectItem const& item) -> void {
  if (ImGui::MenuItem("Rename", nullptr, false, CanRename(item))) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kBeginRename,
      .target = item
    };
  }

  if (ImGui::MenuItem("Delete", nullptr, false, CanDelete(item))) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kDelete,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Show in Explorer")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kShowInExplorer,
      .target = item
    };
  }

  if (ImGui::MenuItem("Copy GUID")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kCopyGuid,
      .target = item
    };
  }

  if (ImGui::MenuItem("Copy Resource ID")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kCopyResourceId,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Unload")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kUnloadResource,
      .target = item
    };
  }
}


auto ProjectWindow::DrawResourcePackageFileContextMenu(ResourcePackageFileProjectItem const& item) -> void {
  if (ImGui::MenuItem("Reimport")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kReimportFile,
      .target = item
    };
  }

  if (ImGui::MenuItem("Import Settings")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kOpenImportSettings,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Rename", nullptr, false, CanRename(item))) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kBeginRename,
      .target = item
    };
  }

  if (ImGui::MenuItem("Delete", nullptr, false, CanDelete(item))) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kDelete,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Show in Explorer")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kShowInExplorer,
      .target = item
    };
  }

  if (ImGui::MenuItem("Copy GUID")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kCopyGuid,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Unload All Resources")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kUnloadAllResourcesInFile,
      .target = item
    };
  }
}


auto ProjectWindow::DrawSubresourceContextMenu(SubresourceProjectItem const& item) -> void {
  if (ImGui::MenuItem("Copy Resource ID")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kCopyResourceId,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Unload")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kUnloadResource,
      .target = item
    };
  }
}


auto ProjectWindow::CanDrag(ProjectItem const& item) const -> bool {
  return (std::holds_alternative<DirectoryProjectItem>(item) &&
          !IsResourceRootDirectory(std::get<DirectoryProjectItem>(item))) ||
         std::holds_alternative<NativeResourceFileProjectItem>(item) ||
         std::holds_alternative<ResourcePackageFileProjectItem>(item) ||
         std::holds_alternative<SubresourceProjectItem>(item);
}


auto ProjectWindow::CanDropOnto(ProjectItem const& item) -> bool {
  return std::holds_alternative<DirectoryProjectItem>(item);
}


auto ProjectWindow::DrawDragSource(ProjectItem const& item) const -> void {
  if (!ImGui::BeginDragDropSource()) {
    return;
  }

  std::visit(Overloaded{
    [this](DirectoryProjectItem const& dir_item) {
      auto const path_str{dir_item.path_abs.u8string()};
      auto const path_sv{ToUntypedStdSv(path_str)};
      ImGui::SetDragDropPayload(kDirNodeDragDropTypeStr.data(), path_sv.data(), path_sv.size());
    },
    [this](NativeResourceFileProjectItem const& res_item) {
      auto const obj{app_->GetResourceManager().GetOrLoad(ResourceId{res_item.guid, 0})};

      if (!obj) {
        spdlog::error("Failed to load native resource when creating drag source. Ignoring.");
        return;
      }

      ObjectDragDropPayload const payload{
        .ptr = obj
      };
      ImGui::SetDragDropPayload(ObjectDragDropPayload::kTypeStr.data(), &payload, sizeof(payload));
    },
    [this](ResourcePackageFileProjectItem const& res_pack_item) {
      ImGui::SetDragDropPayload(kResPackNodeDragDropTypeStr.data(), &res_pack_item.guid, sizeof(res_pack_item.guid));
    },
    [this](SubresourceProjectItem const& subres_item) {
      auto const obj{app_->GetResourceManager().GetOrLoad(subres_item.id)};

      if (!obj) {
        spdlog::error("Failed to load subresource when creating drag source. Ignoring.");
        return;
      }

      ObjectDragDropPayload const payload{
        .ptr = obj
      };
      ImGui::SetDragDropPayload(ObjectDragDropPayload::kTypeStr.data(), &payload, sizeof(payload));
    }
  }, item);

  ImGui::EndDragDropSource();
}


auto ProjectWindow::DrawDropTarget(ProjectItem const& item) -> void {
  if (!ImGui::BeginDragDropTarget()) {
    return;
  }

  std::visit(Overloaded{
    [this](DirectoryProjectItem const& dir_target) {
      auto const* payload{ImGui::GetDragDropPayload()};

      if (!payload) {
        spdlog::error("Couldn't get payload on drag drop target. Ignoring.");
        return;
      }

      if (payload->IsDataType(kDirNodeDragDropTypeStr.data())) {
        std::string_view const src_abs_sv{
          static_cast<char const*>(payload->Data), clamp_cast<std::size_t>(payload->DataSize)
        };

        std::filesystem::path const src_abs{ToUtf8StdSv(src_abs_sv)};

        if (std::error_code ec;
          IsResourceRootDirectory(src_abs) ||
          IsSubpath(dir_target.path_abs, src_abs) ||
          equivalent(src_abs.parent_path(), dir_target.path_abs, ec) ||
          ec) {
          return;
        }

        ImGui::AcceptDragDropPayload(kDirNodeDragDropTypeStr.data());

        if (payload->IsDelivery()) {
          move_to_folder_ctx_ = MoveToFolderContext{
            .src_abs = src_abs
          };

          pending_command_ = ProjectCommand{
            .kind = ProjectCommandKind::kMoveToFolder,
            .target = dir_target
          };
        }

        return;
      }

      if (payload->IsDataType(kResPackNodeDragDropTypeStr.data())) {
        auto const* const guid{static_cast<Guid const*>(payload->Data)};
        auto const src_res_dir_rel{resource_db_->GuidToPath(*guid)};

        if (src_res_dir_rel.empty()) {
          spdlog::error("Failed to get resource package path during drop. Ignoring.");
          DisplayError("Failed to move file.");
          return;
        }

        auto const src_abs{resource_db_->GetResourceDirectoryAbsolutePath() / src_res_dir_rel};

        if (std::error_code ec; equivalent(src_abs.parent_path(), dir_target.path_abs, ec) || ec) {
          return;
        }

        ImGui::AcceptDragDropPayload(kResPackNodeDragDropTypeStr.data());

        if (payload->IsDelivery()) {
          move_to_folder_ctx_ = MoveToFolderContext{
            .src_abs = src_abs
          };

          pending_command_ = ProjectCommand{
            .kind = ProjectCommandKind::kMoveToFolder,
            .target = dir_target
          };
        }

        return;
      }

      if (payload->IsDataType(ObjectDragDropPayload::kTypeStr.data())) {
        auto const res{rttr::rttr_cast<NativeResource*>(static_cast<ObjectDragDropPayload const*>(payload->Data)->ptr)};

        if (!res) {
          spdlog::info("Received non-native resource object payload in directory drop target. Ignoring.");
          return;
        }

        if (!resource_db_->IsResourceEditable(res->GetId())) {
          spdlog::info("Received non-editable native resource object payload in directory drop target. Ignoring.");
          return;
        }

        auto const file_info{resource_db_->GetFileInfo(res->GetId().GetGuid())};

        if (!file_info) {
          spdlog::error("Failed to get file info for drop target resource. Ignoring.");
          DisplayError("Failed to move file.");
          return;
        }

        auto const src_abs{resource_db_->GetResourceDirectoryAbsolutePath() / file_info->src_path_res_dir_rel};

        if (std::error_code ec; equivalent(src_abs.parent_path(), dir_target.path_abs, ec) || ec) {
          return;
        }

        ImGui::AcceptDragDropPayload(ObjectDragDropPayload::kTypeStr.data());

        if (payload->IsDelivery()) {
          move_to_folder_ctx_ = MoveToFolderContext{
            .src_abs = src_abs
          };

          pending_command_ = ProjectCommand{
            .kind = ProjectCommandKind::kMoveToFolder,
            .target = dir_target
          };
        }
      }
    },
    []([[maybe_unused]] NativeResourceFileProjectItem const& res_item) {
      spdlog::error("Tried to drop onto a native resource file. Ignoring.");
    },
    []([[maybe_unused]] ResourcePackageFileProjectItem const& res_pack_item) {
      spdlog::error("Tried to drop onto a resource package file. Ignoring.");
    },
    []([[maybe_unused]] SubresourceProjectItem const& subres_item) {
      spdlog::error("Tried to drop onto a subresource. Ignoring.");
    }
  }, item);

  ImGui::EndDragDropTarget();
}


auto ProjectWindow::CanRename(ProjectItem const& item) const -> bool {
  return std::visit(Overloaded{
    [this]([[maybe_unused]] DirectoryProjectItem const& dir_item) {
      // Don't allow renaming the root resource directory.
      return !IsResourceRootDirectory(dir_item);
    },
    []([[maybe_unused]] NativeResourceFileProjectItem const& res_item) {
      return true;
    },
    []([[maybe_unused]] ResourcePackageFileProjectItem const& res_pack_item) {
      return true;
    },
    []([[maybe_unused]] SubresourceProjectItem const& subres_item) {
      return false;
    }
  }, item);
}


auto ProjectWindow::IsRenaming(ProjectItem const& item) const -> bool {
  return rename_ctx_ && rename_ctx_->target == item;
}


auto ProjectWindow::IsValidSingleFilename(std::string_view const name) -> bool {
  if (name.empty()) {
    return false;
  }

  if (name == "." || name == "..") {
    return false;
  }

  // Reject name if it contains a path separator.
  if (std::filesystem::path{name}.has_parent_path()) {
    return false;
  }

  return true;
}


auto ProjectWindow::DrawRenameInput() -> void {
  if (!rename_ctx_) {
    return;
  }

  ImGui::SameLine();
  ImGui::PushID("rename_input");

  if (rename_ctx_->focus_requested) {
    ImGui::SetKeyboardFocusHere();
    rename_ctx_->focus_requested = false;
  }

  auto constexpr flags{ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll};

  auto const pressed_enter{ImGui::InputText("##Rename", &rename_ctx_->new_name, flags)};
  auto const pressed_escape{ImGui::IsKeyPressed(ImGuiKey_Escape)};
  auto const deactivated{ImGui::IsItemDeactivated()};

  if (!rename_ctx_->error_msg.empty()) {
    ImGui::TextColored(ImVec4{1.0f, 0.0f, 0.0f, 1.0f}, "%s", rename_ctx_->error_msg.c_str());
    spdlog::error("Rename error: {}", rename_ctx_->error_msg);
  }

  if (pressed_escape) {
    pending_rename_action_ = PendingRenameAction::kCancel;
  } else if (pressed_enter || deactivated) {
    pending_rename_action_ = PendingRenameAction::kCommit;
  }

  ImGui::PopID();
}


auto ProjectWindow::HandleRenameShortcut() -> void {
  if (!selected_item_) {
    return;
  }

  if (!CanRename(*selected_item_)) {
    return;
  }

  if (ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kBeginRename,
      .target = *selected_item_
    };
  }
}


auto ProjectWindow::ExecutePendingRenameAction() -> void {
  if (!pending_rename_action_) {
    return;
  }

  auto const action{std::exchange(pending_rename_action_, std::nullopt)};

  switch (*action) {
    case PendingRenameAction::kCommit: {
      CommitRename();
      break;
    }

    case PendingRenameAction::kCancel: {
      rename_ctx_.reset();
      break;
    }
  }
}


auto ProjectWindow::CommitRename() -> void {
  if (!rename_ctx_) {
    spdlog::error("Cannot commit rename without an active rename context.");
    return;
  }

  auto const new_name{Trim(rename_ctx_->new_name)};

  if (!IsValidSingleFilename(new_name)) {
    rename_ctx_->error_msg = "Invalid name.";
    rename_ctx_->focus_requested = true;
    return;
  }

  auto const success{
    std::visit(Overloaded{
      [&](DirectoryProjectItem const& item) {
        return CommitDirectoryRename(item, new_name);
      },

      [&](NativeResourceFileProjectItem const& item) {
        return CommitResourceFileRename(item.guid, new_name);
      },

      [&](ResourcePackageFileProjectItem const& item) {
        return CommitResourceFileRename(item.guid, new_name);
      },

      [this]([[maybe_unused]] SubresourceProjectItem const& item) {
        return false;
      }
    }, rename_ctx_->target)
  };

  if (success) {
    rename_ctx_.reset();
  }
}


auto ProjectWindow::CommitDirectoryRename(DirectoryProjectItem const& item, std::string_view const new_name) -> bool {
  if (item.path_abs.empty()) {
    return false;
  }

  auto const& src_path_abs{item.path_abs};
  auto const dst_path_abs{src_path_abs.parent_path() / new_name};

  if (src_path_abs == dst_path_abs) {
    return true;
  }

  if (std::filesystem::exists(dst_path_abs)) {
    rename_ctx_->error_msg = "A folder or resource with that name already exists.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  if (!resource_db_->MoveDirectory(src_path_abs, dst_path_abs)) {
    rename_ctx_->error_msg = "Failed to rename folder.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  selected_item_ = DirectoryProjectItem{
    .path_abs = dst_path_abs
  };

  return true;
}


auto ProjectWindow::CommitResourceFileRename(Guid const& guid, std::string_view const new_name) -> bool {
  auto info = resource_db_->GetFileInfo(guid);

  if (!info) {
    rename_ctx_->error_msg = "Resource file no longer exists.";
    return false;
  }

  auto const& old_path_res_dir_rel{info->src_path_res_dir_rel};

  auto const new_filename{
    std::string{new_name}.append(ToUntypedStdSv(old_path_res_dir_rel.extension().u8string()))
  };

  auto const new_path_res_dir_rel{old_path_res_dir_rel.parent_path() / new_filename};

  if (old_path_res_dir_rel == new_path_res_dir_rel) {
    return true;
  }

  auto const dst_path_abs{
    resource_db_->GetResourceDirectoryAbsolutePath() / new_path_res_dir_rel
  };

  if (std::filesystem::exists(dst_path_abs)) {
    rename_ctx_->error_msg = "A file or folder with that name already exists.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  if (!resource_db_->MoveResourceFile(guid, new_path_res_dir_rel)) {
    rename_ctx_->error_msg = "Failed to rename resource file.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  return true;
}


auto ProjectWindow::IsResourceRootDirectory(DirectoryProjectItem const& item) const -> bool {
  return IsResourceRootDirectory(item.path_abs);
}


auto ProjectWindow::IsResourceRootDirectory(std::filesystem::path const& path_abs) const -> bool {
  std::error_code ec;
  return exists(path_abs) && equivalent(path_abs, resource_db_->GetResourceDirectoryAbsolutePath(), ec) && !ec;
}


auto ProjectWindow::CanDelete(ProjectItem const& item) const -> bool {
  return std::visit(Overloaded{
    [this](DirectoryProjectItem const& dir_item) {
      return !IsResourceRootDirectory(dir_item);
    },
    []([[maybe_unused]] NativeResourceFileProjectItem const& native_res_item) {
      return true;
    },
    []([[maybe_unused]] ResourcePackageFileProjectItem const& res_package_item) {
      return true;
    },
    []([[maybe_unused]] SubresourceProjectItem const& subres_item) {
      return false;
    }
  }, item);
}


auto ProjectWindow::ExecutePendingCommand() -> void {
  if (!pending_command_) {
    return;
  }

  auto const command = std::exchange(pending_command_, std::nullopt);

  switch (command->kind) {
    case ProjectCommandKind::kImportFilesIntoFolder: {
      ExecuteImport(command->target);
      break;
    }

    case ProjectCommandKind::kCreateFolder: {
      ExecuteCreateFolder(command->target);
      break;
    }

    case ProjectCommandKind::kCreateMaterial: {
      ExecuteCreateMaterial(command->target);
      break;
    }

    case ProjectCommandKind::kCreateScene: {
      ExecuteCreateScene(command->target);
      break;
    }

    case ProjectCommandKind::kBeginRename: {
      BeginRename(command->target);
      break;
    }

    case ProjectCommandKind::kDelete: {
      ExecuteDelete(command->target);
      break;
    }

    case ProjectCommandKind::kShowInExplorer: {
      ExecuteShowInExplorer(command->target);
      break;
    }

    case ProjectCommandKind::kReimportFile: {
      ExecuteReimport(command->target);
      break;
    }

    case ProjectCommandKind::kOpenImportSettings: {
      OpenImportSettings(command->target);
      break;
    }

    case ProjectCommandKind::kCopyGuid: {
      ExecuteCopyGuid(command->target);
      break;
    }

    case ProjectCommandKind::kCopyResourceId: {
      ExecuteCopyResourceId(command->target);
      break;
    }

    case ProjectCommandKind::kUnloadResource: {
      ExecuteUnloadResource(command->target);
      break;
    }

    case ProjectCommandKind::kUnloadAllResourcesInFile: {
      ExecuteUnloadAllResourcesInFile(command->target);
      break;
    }

    case ProjectCommandKind::kMoveToFolder: {
      ExecuteMoveToFolder(command->target);
      break;
    }
  }
}


auto ProjectWindow::ExecuteImport(ProjectItem const& target) const -> void {
  auto const target_dir_abs{
    std::visit(Overloaded{
      [](DirectoryProjectItem const& folder) -> std::optional<std::filesystem::path> {
        return folder.path_abs;
      },
      []([[maybe_unused]] ResourcePackageFileProjectItem const& file) -> std::optional<std::filesystem::path> {
        spdlog::error("Import command received a resource package instead of a directory as target, ignoring.");
        return std::nullopt;
      },
      []([[maybe_unused]] NativeResourceFileProjectItem const& file) -> std::optional<std::filesystem::path> {
        spdlog::error("Import command received a native resource instead of a directory as target, ignoring.");
        return std::nullopt;
      },
      []([[maybe_unused]] SubresourceProjectItem const& resource) -> std::optional<std::filesystem::path> {
        spdlog::error("Import command received a subresource instead of a directory as target, ignoring.");
        return std::nullopt;
      }
    }, target)
  };

  NFD::UniquePathSet import_paths;

  if (OpenDialogMultiple(import_paths, static_cast<nfdnfilteritem_t*>(nullptr)) != NFD_OKAY) {
    return;
  }

  nfdpathsetsize_t import_path_count{0};

  if (NFD::PathSet::Count(import_paths, import_path_count) != NFD_OKAY) {
    spdlog::error("Failed to count selected import paths.");
    return;
  }

  for (nfdpathsetsize_t i{0}; i < import_path_count; i++) {
    NFD::UniquePathSetPathN src_path_abs_str;

    if (NFD::PathSet::GetPath(import_paths, i, src_path_abs_str) != NFD_OKAY) {
      spdlog::error("Failed to get selected import path at index {}.", i);
      continue;
    }

    std::filesystem::path const src_path_abs{src_path_abs_str.get()};
    auto const dst_path_abs{GenerateUniquePath(*target_dir_abs / src_path_abs.filename())};

    if (!exists(src_path_abs)) {
      auto const err_msg{
        std::format("Failed to import {}. File does not exist.", ToUntypedStdSv(src_path_abs.u8string()))
      };
      spdlog::error(err_msg);
      DisplayError(err_msg);
      return;
    }

    if (exists(dst_path_abs)) {
      auto const err_msg{
        std::format("Failed to import {}. File already exists.", ToUntypedStdSv(dst_path_abs.u8string()))
      };
      spdlog::error(err_msg);
      DisplayError(err_msg);
      return;
    }

    std::error_code ec;
    copy_file(src_path_abs, dst_path_abs, ec);

    if (ec) {
      auto const err_msg{
        std::format("Failed to import {}. Copy failed.", ToUntypedStdSv(src_path_abs.u8string()))
      };
      spdlog::error(err_msg);
      DisplayError(err_msg);
      return;
    }

    resource_db_->Refresh();
  }
}


auto ProjectWindow::ExecuteCreateFolder(ProjectItem const& target) const -> void {
  auto const* const dir_target{std::get_if<DirectoryProjectItem>(&target)};

  if (!dir_target) {
    spdlog::error("Tried to create folder inside a project item that is not a directory. Ignoring.");
    return;
  }

  auto const new_folder_path_abs{GenerateUniquePath(dir_target->path_abs / "New Folder")};

  std::error_code err;
  create_directory(new_folder_path_abs, err);

  if (err) {
    auto const err_msg{std::format("Failed to create directory: {}.", ToUntypedStdSv(new_folder_path_abs.u8string()))};
    spdlog::error(err_msg);
    DisplayError(err_msg);
    return;
  }

  resource_db_->Refresh();
}


auto ProjectWindow::ExecuteCreateMaterial(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    [this](DirectoryProjectItem const& item) {
      auto mtl{Create<Material>()};
      auto const mtl_path_abs{GenerateUniquePath(item.path_abs / "New Material.mtl")};

      auto created_mtl{app_->GetResourceDatabase().SaveResourceToFile(std::move(mtl), mtl_path_abs)};

      if (!created_mtl) {
        auto const err_msg{
          std::format("Failed to create new material at {}.", ToUntypedStdSv(mtl_path_abs.u8string()))
        };
        spdlog::error(err_msg);
        DisplayError(err_msg);
        return;
      }

      SelectItem(NativeResourceFileProjectItem{
        created_mtl->GetId().GetGuid()
      });
    },
    []([[maybe_unused]] NativeResourceFileProjectItem const& item) {
      spdlog::error("Tried to create material at a native resource file target. Ignoring.");
    },
    []([[maybe_unused]] ResourcePackageFileProjectItem const& item) {
      spdlog::error("Tried to create material at a resource package file target. Ignoring.");
    },
    []([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried to create material and a subresource target. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteCreateScene(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    [this](DirectoryProjectItem const& item) {
      auto scene{Create<Scene>()};
      auto const scene_path_abs{GenerateUniquePath(item.path_abs / "New Scene.scene")};

      auto created_scene{app_->GetResourceDatabase().SaveResourceToFile(std::move(scene), scene_path_abs)};

      if (!created_scene) {
        auto const err_msg{
          std::format("Failed to create new scene at {}.", ToUntypedStdSv(scene_path_abs.u8string()))
        };
        spdlog::error(err_msg);
        DisplayError(err_msg);
        return;
      }

      SelectItem(NativeResourceFileProjectItem{
        created_scene->GetId().GetGuid()
      });
    },
    []([[maybe_unused]] NativeResourceFileProjectItem const& item) {
      spdlog::error("Tried to create scene at a native resource file target. Ignoring.");
    },
    []([[maybe_unused]] ResourcePackageFileProjectItem const& item) {
      spdlog::error("Tried to create scene at a resource package file target. Ignoring.");
    },
    []([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried to create scene and a subresource target. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::BeginRename(ProjectItem const& target) -> void {
  if (!CanRename(target)) {
    spdlog::error("Tried renaming a project item that cannot be renamed. Ignoring.");
    return;
  }

  auto const name{
    std::visit(Overloaded{
      [](DirectoryProjectItem const& item) -> std::optional<std::string> {
        if (item.path_abs.empty()) {
          return std::nullopt;
        }
        return item.path_abs.filename().string();
      },

      [this](NativeResourceFileProjectItem const& item) -> std::optional<std::string> {
        auto const info{resource_db_->GetFileInfo(item.guid)};
        if (!info) {
          return std::nullopt;
        }
        return info->src_path_res_dir_rel.stem().string();
      },

      [this](ResourcePackageFileProjectItem const& item) -> std::optional<std::string> {
        auto const info{resource_db_->GetFileInfo(item.guid)};
        if (!info) {
          return std::nullopt;
        }
        return info->src_path_res_dir_rel.stem().string();
      },

      [](SubresourceProjectItem const&) -> std::optional<std::string> {
        return std::nullopt;
      }
    }, target)
  };

  if (!name) {
    spdlog::error("Failed to get name to rename. Aborting.");
    return;
  }

  rename_ctx_ = RenameContext{
    .new_name = *name,
    .target = target,
    .focus_requested = true
  };
}


auto ProjectWindow::ExecuteDelete(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    [this](DirectoryProjectItem const& item) {
      ExecuteDeleteDirectory(item);
    },
    [this](NativeResourceFileProjectItem const& item) {
      ExecuteDeleteResourceFile(item.guid);
    },
    [this](ResourcePackageFileProjectItem const& item) {
      ExecuteDeleteResourceFile(item.guid);
    },
    []([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried deleting a subresource. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteDeleteDirectory(DirectoryProjectItem const& target) -> void {
  if (IsResourceRootDirectory(target)) {
    auto constexpr err_msg{"Tried deleting resource root directory. Ignoring."};
    spdlog::error(err_msg);
    DisplayError(err_msg);
    return;
  }

  if (!app_->GetResourceDatabase().DeleteDirectory(target.path_abs)) {
    auto const err_msg{std::format("Failed to delete folder: {}", ToUntypedStdSv(target.path_abs.u8string()))};
    spdlog::error(err_msg);
    DisplayError(err_msg);
    return;
  }

  ClearSelection();
}


auto ProjectWindow::ExecuteDeleteResourceFile(Guid const& target) -> void {
  if (!app_->GetResourceDatabase().GetFileInfo(target)) {
    spdlog::error("Tried deleting resource file with GUID {} that does not exist. Ignoring.", target.ToString());
    return;
  }

  app_->GetResourceDatabase().DeleteResourceFile(target);
  ClearSelection();
}


auto ProjectWindow::ExecuteShowInExplorer(ProjectItem const& target) const -> void {
  std::visit(Overloaded{
    [this](DirectoryProjectItem const& item) {
      if (!ShowInFileExplorer(item.path_abs)) {
        spdlog::error("Failed to show directory in explorer.");
        DisplayError("Failed to open File Explorer.");
      }
    },
    [this](NativeResourceFileProjectItem const& item) {
      auto const path_res_dir_rel{resource_db_->GuidToPath(item.guid)};
      if (path_res_dir_rel.empty()) {
        spdlog::error("Tried showing a native resource file in explorer, but failed to get its path.");
        DisplayError("Failed to get file path.");
        return;
      }
      if (!ShowInFileExplorer(resource_db_->GetResourceDirectoryAbsolutePath() / path_res_dir_rel)) {
        spdlog::error("Failed to show native resource file in explorer.");
        DisplayError("Failed to open File Explorer.");
      }
    },
    [this](ResourcePackageFileProjectItem const& item) {
      auto const path_res_dir_rel{resource_db_->GuidToPath(item.guid)};
      if (path_res_dir_rel.empty()) {
        spdlog::error("Tried showing a resource package file in explorer, but failed to get its path.");
        DisplayError("Failed to get file path.");
        return;
      }
      if (!ShowInFileExplorer(resource_db_->GetResourceDirectoryAbsolutePath() / path_res_dir_rel)) {
        spdlog::error("Failed to show resource package file in explorer.");
        DisplayError("Failed to open File Explorer.");
      }
    },
    []([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried showing a subresource in explorer. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteReimport(ProjectItem const& target) const -> void {
  std::visit(Overloaded{
    []([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried reimporting a folder. Ignoring.");
    },
    []([[maybe_unused]] NativeResourceFileProjectItem const& item) {
      spdlog::error("Tried reimporting a native resource. Ignoring.");
    },
    [this](ResourcePackageFileProjectItem const& item) {
      auto const file_info{resource_db_->GetFileInfo(item.guid)};

      if (!file_info) {
        spdlog::error("Tried to reimport but failed to get file info.");
        DisplayError("Failed to reimport.");
        return;
      }

      auto const src_path_abs{resource_db_->GetResourceDirectoryAbsolutePath() / file_info->src_path_res_dir_rel};
      auto const importer{ResourceDB::GetImporterForResourceFile(src_path_abs)};

      if (!importer) {
        spdlog::error("Tried to reimport but failed to get importer.");
        DisplayError("Failed to reimport.");
        return;
      }

      if (!resource_db_->ImportResourceFile(file_info->src_path_res_dir_rel, importer.get())) {
        auto constexpr err_msg{"Failed to reimport."};
        spdlog::error(err_msg);
        DisplayError(err_msg);
      }
    },
    []([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried reimporting a subresource. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::OpenImportSettings(ProjectItem const& target) -> void {
  // TODO implement
}


auto ProjectWindow::ExecuteCopyGuid(ProjectItem const& target) const -> void {
  std::visit(Overloaded{
    []([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried copying GUID of a directory. Ignoring.");
    },
    [this](NativeResourceFileProjectItem const& item) {
      if (!CopyToClipboard(item.guid.ToString())) {
        spdlog::error("Failed to copy GUID to clipboard.");
      }
    },
    [this](ResourcePackageFileProjectItem const& item) {
      if (!CopyToClipboard(item.guid.ToString())) {
        spdlog::error("Failed to copy GUID to clipboard.");
      }
    },
    []([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried copying GUID of a subresource. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteCopyResourceId(ProjectItem const& target) const -> void {
  std::visit(Overloaded{
    [this]([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried copying resource ID of a directory. Ignoring.");
    },
    [this](NativeResourceFileProjectItem const& item) {
      if (!CopyToClipboard(ResourceId{item.guid, 0}.ToString())) {
        spdlog::error("Failed to copy resource ID to clipboard.");
      }
    },
    [this]([[maybe_unused]] ResourcePackageFileProjectItem const& item) {
      spdlog::error("Tried copying resource ID of a resource package file. Ignoring.");
    },
    [this](SubresourceProjectItem const& item) {
      if (!CopyToClipboard(item.id.ToString())) {
        spdlog::error("Failed to copy resource ID to clipboard.");
      }
    }
  }, target);
}


auto ProjectWindow::ExecuteUnloadResource(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    []([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried unloading a directory. Ignoring.");
    },
    [this](NativeResourceFileProjectItem const& item) {
      ExecuteUnloadResourceById(ResourceId{item.guid, 0});
    },
    [this]([[maybe_unused]] ResourcePackageFileProjectItem const& item) {
      spdlog::error("Tried unloading a resource package file. Ignoring.");
    },
    [this](SubresourceProjectItem const& item) {
      ExecuteUnloadResourceById(item.id);
    }
  }, target);
}


auto ProjectWindow::ExecuteUnloadAllResourcesInFile(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    [this]([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried unloading all resources of a directory. Ignoring.");
    },
    [this]([[maybe_unused]] NativeResourceFileProjectItem const& item) {
      spdlog::error("Tried unloading all resources of a native resource file. Ignoring.");
    },
    [this](ResourcePackageFileProjectItem const& item) {
      std::vector<ObserverPtr<ResourceDB::ResourceInfo const>> subresources;
      resource_db_->GetResourcesInFile(item.guid, subresources);
      for (auto const& subresource : subresources) {
        ExecuteUnloadResourceById(subresource->id);
      }
    },
    [this]([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried unloading all resources in a subresource. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteUnloadResourceById(ResourceId const& target) -> void {
  App::Instance().GetResourceManager().Unload(target);
  ClearSelection();
}


auto ProjectWindow::ExecuteMoveToFolder(ProjectItem const& target) -> void {
  if (!move_to_folder_ctx_) {
    spdlog::error("Tried running move to folder command without context. Ignoring.");
    return;
  }

  auto const move_to_folder_ctx{std::exchange(move_to_folder_ctx_, std::nullopt)};

  std::visit(Overloaded{
    [this, &move_to_folder_ctx](DirectoryProjectItem const& dir_target) {
      std::error_code ec;
      auto const src_status{std::filesystem::status(move_to_folder_ctx->src_abs, ec)};

      if (ec) {
        spdlog::error("Tried moving to folder but failed to get status of source file. Ignoring.");
        return;
      }

      if (!exists(src_status)) {
        spdlog::error("Tried moving to folder but the source path does not exist. Ignoring.");
        return;
      }

      auto const dst_abs{dir_target.path_abs / move_to_folder_ctx->src_abs.filename()};

      if ((exists(dst_abs) && equivalent(dst_abs, move_to_folder_ctx->src_abs, ec)) || ec) {
        spdlog::info("Move to folder command source and destination are the same. Ignoring.");
        return;
      }

      if (is_directory(src_status)) {
        if (IsSubpath(dst_abs, move_to_folder_ctx->src_abs)) {
          spdlog::error("Tried moving a folder into its own subdirectory. Ignoring.");
          return;
        }

        if (!resource_db_->MoveDirectory(move_to_folder_ctx->src_abs, dst_abs)) {
          spdlog::error("Failed to move directory {} to {}.", ToUntypedStdSv(move_to_folder_ctx->src_abs.u8string()),
            ToUntypedStdSv(dst_abs.u8string()));
          DisplayError("Failed to move folder.");
        }
      } else {
        if (!IsSubpath(move_to_folder_ctx->src_abs, resource_db_->GetResourceDirectoryAbsolutePath())) {
          spdlog::error("Tried moving a file that is not in resource directory. Ignoring.");
          return;
        }

        auto const src_res_dir_rel{
          relative(move_to_folder_ctx->src_abs, resource_db_->GetResourceDirectoryAbsolutePath())
        };

        auto const dst_res_dir_rel{
          relative(dst_abs, resource_db_->GetResourceDirectoryAbsolutePath())
        };

        auto const guid{resource_db_->PathToGuid(src_res_dir_rel)};

        if (!guid.IsValid()) {
          spdlog::error("Tried moving a file that is not in resource database. Ignoring.");
          return;
        }

        if (!resource_db_->MoveResourceFile(guid, dst_res_dir_rel)) {
          spdlog::error("Failed to move resource file {} to {}.",
            ToUntypedStdSv(move_to_folder_ctx->src_abs.u8string()),
            ToUntypedStdSv(dst_abs.u8string()));
          DisplayError("Failed to move resource file.");
        }
      }
    },
    []([[maybe_unused]] NativeResourceFileProjectItem const& item) {
      spdlog::error("Tried moving into a native resource file. Ignoring.");
    },
    []([[maybe_unused]] ResourcePackageFileProjectItem const& item) {
      spdlog::error("Tried moving into a resource package file. Ignoring.");
    },
    []([[maybe_unused]] SubresourceProjectItem const& item) {
      spdlog::error("Tried moving into a subresource. Ignoring.");
    }
  }, target);
}
}
