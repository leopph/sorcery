#include "ProjectWindow.hpp"

#include <optional>
#include <queue>

#include <imgui.h>
#include <nfd.hpp>
#include <spdlog/spdlog.h>

#include "../editor_app.hpp"
#include "../drawers/editor_drawer_registry.hpp"
#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/util.hpp"
#include "sorcery/host/platform.hpp"
#include "sorcery/assets/Material.hpp"


namespace sorcery::mage {
ProjectWindow::ProjectWindow(EditorApp& context, AssetDB& asset_db, EditorDrawerRegistry& drawer_registry) :
  app_{&context},
  asset_db_{&asset_db},
  drawer_registry_{&drawer_registry},
  database_changed_listener_{
    asset_db.OnDatabaseChanged.add_listener([this] { should_rebuild_hierarchy_on_next_draw_ = true; })
  } {}


ProjectWindow::~ProjectWindow() {
  asset_db_->OnDatabaseChanged.remove_listener(database_changed_listener_);
}


auto ProjectWindow::RebuildHierarchy() -> void {
  spdlog::debug("Starting project window hierarchy rebuild.");

  root_node_ = ProjectTreeNode{
    .item = DirectoryProjectItem{.path_abs = asset_db_->GetAssetDirectoryAbsolutePath()},
    .display_name = asset_db_->GetAssetDirectoryAbsolutePath().filename().string(),
    .imgui_id = std::format("##{}", asset_db_->GetAssetDirectoryAbsolutePath().string()),
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
        if (AssetDB::IsMetaFile(entry.path())) {
          continue;
        }

        auto const asset_dir_rel{relative(entry.path(), asset_db_->GetAssetDirectoryAbsolutePath())};
        auto const guid{asset_db_->PathToGuid(asset_dir_rel)};

        if (!guid.IsValid()) {
          continue;
        }

        auto const file_info{asset_db_->GetFileInfo(guid)};

        if (!file_info) {
          continue;
        }

        if (file_info->is_native_asset) {
          current_node->children.emplace_back(ProjectTreeNode{
            .item = NativeAssetFileProjectItem{.guid = guid},
            .display_name = entry.path().filename().string(),
            .imgui_id = std::format("##{}", entry.path().string()),
            .children = {}
          });
        } else {
          std::vector<ObserverPtr<AssetDB::AssetInfo const>> subassets;
          asset_db_->GetAssetsInFile(guid, subassets);

          std::vector<ProjectTreeNode> subasset_nodes;

          for (auto const& subasset : subassets) {
            subasset_nodes.emplace_back(ProjectTreeNode{
              .item = SubassetProjectItem{.id = subasset->id},
              .display_name = subasset->name,
              .imgui_id = std::format("##{}", std::string{subasset->id}),
              .children = {}
            });
          }

          current_node->children.emplace_back(ProjectTreeNode{
            .item = AssetPackageFileProjectItem{.guid = guid},
            .display_name = entry.path().filename().string(),
            .imgui_id = std::format("##{}", entry.path().string()),
            .children = std::move(subasset_nodes)
          });
        }
      }
    }

    // Sort children by name.
    // Folders first, then files.
    // Subassets are not sorted.
    if (!std::holds_alternative<AssetPackageFileProjectItem>(current_node->item)) {
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
    DrawImportSettingsDialog();
  }
  ImGui::End();

  HandleRenameShortcut();
  ExecutePendingCommand();
  ExecutePendingRenameAction();
}


auto ProjectWindow::IsDirectory(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<DirectoryProjectItem>(node.item);
}


auto ProjectWindow::IsAssetPackageFile(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<AssetPackageFileProjectItem>(node.item);
}


auto ProjectWindow::IsNativeAssetFile(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<NativeAssetFileProjectItem>(node.item);
}


auto ProjectWindow::IsSubasset(ProjectTreeNode const& node) -> bool {
  return std::holds_alternative<SubassetProjectItem>(node.item);
}


auto ProjectWindow::ShouldDrawAsTree(ProjectTreeNode const& node) -> bool {
  return IsDirectory(node) || IsAssetPackageFile(node);
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

    [this](NativeAssetFileProjectItem const& file) {
      app_->SetSelectedObject(App::Instance().GetAssetManager().Resolve(AssetId{file.guid, 0}));
    },

    [this](AssetPackageFileProjectItem const&) {
      app_->SetSelectedObject(nullptr);
    },

    [this](SubassetProjectItem const& asset) {
      app_->SetSelectedObject(App::Instance().GetAssetManager().Resolve(asset.id));
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
            asset_db_->GetAssetDirectoryAbsolutePath()).empty()) {
        ClearSelection();
      }
    },
    [&](NativeAssetFileProjectItem const& item) {
      if (!asset_db_->GetFileInfo(item.guid)) {
        ClearSelection();
      }
    },
    [&](AssetPackageFileProjectItem const& item) {
      if (!asset_db_->GetFileInfo(item.guid)) {
        ClearSelection();
      }
    },
    [&](SubassetProjectItem const& item) {
      if (!asset_db_->GetAssetInfo(item.id)) {
        if (asset_db_->GetFileInfo(item.id.GetGuid())) {
          selected_item_ = AssetPackageFileProjectItem{item.id.GetGuid()};
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
    [this](NativeAssetFileProjectItem const& file) {
      DrawNativeAssetFileContextMenu(file);
    },
    [this](AssetPackageFileProjectItem const& file) {
      DrawAssetPackageFileContextMenu(file);
    },
    [this](SubassetProjectItem const& asset) {
      DrawSubassetContextMenu(asset);
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

  if (ImGui::MenuItem("Import New Asset")) {
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


auto ProjectWindow::DrawNativeAssetFileContextMenu(NativeAssetFileProjectItem const& item) -> void {
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

  if (ImGui::MenuItem("Copy Asset ID")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kCopyAssetId,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Unload")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kUnloadAsset,
      .target = item
    };
  }
}


auto ProjectWindow::DrawAssetPackageFileContextMenu(AssetPackageFileProjectItem const& item) -> void {
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

  if (ImGui::MenuItem("Unload All Assets")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kUnloadAllAssetsInFile,
      .target = item
    };
  }
}


auto ProjectWindow::DrawSubassetContextMenu(SubassetProjectItem const& item) -> void {
  if (ImGui::MenuItem("Copy Asset ID")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kCopyAssetId,
      .target = item
    };
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Unload")) {
    pending_command_ = ProjectCommand{
      .kind = ProjectCommandKind::kUnloadAsset,
      .target = item
    };
  }
}


auto ProjectWindow::CanDrag(ProjectItem const& item) const -> bool {
  return (std::holds_alternative<DirectoryProjectItem>(item) &&
          !IsAssetRootDirectory(std::get<DirectoryProjectItem>(item))) ||
         std::holds_alternative<NativeAssetFileProjectItem>(item) ||
         std::holds_alternative<AssetPackageFileProjectItem>(item) ||
         std::holds_alternative<SubassetProjectItem>(item);
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
      auto const path_sv{ToUntypedSv(path_str)};
      ImGui::SetDragDropPayload(kDirNodeDragDropTypeStr.data(), path_sv.data(), path_sv.size());
    },
    [this](NativeAssetFileProjectItem const& asset_item) {
      auto const obj{app_->GetAssetManager().Resolve(AssetId{asset_item.guid, 0})};

      if (!obj) {
        spdlog::error("Failed to load native asset when creating drag source. Ignoring.");
        return;
      }

      ObjectDragDropPayload const payload{
        .ptr = obj.Get().Get()
      };
      ImGui::SetDragDropPayload(ObjectDragDropPayload::kTypeStr.data(), &payload, sizeof(payload));
    },
    [this](AssetPackageFileProjectItem const& asset_pack_item) {
      ImGui::SetDragDropPayload(kAssetPackNodeDragDropTypeStr.data(), &asset_pack_item.guid, sizeof(asset_pack_item.guid));
    },
    [this](SubassetProjectItem const& subres_item) {
      auto const obj{app_->GetAssetManager().Resolve(subres_item.id)};

      if (!obj) {
        spdlog::error("Failed to load subasset when creating drag source. Ignoring.");
        return;
      }

      ObjectDragDropPayload const payload{
        .ptr = obj.Get().Get()
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

        std::filesystem::path const src_abs{ToUtf8Sv(src_abs_sv)};

        if (std::error_code ec;
          IsAssetRootDirectory(src_abs) ||
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

      if (payload->IsDataType(kAssetPackNodeDragDropTypeStr.data())) {
        auto const* const guid{static_cast<Guid const*>(payload->Data)};
        auto const src_asset_dir_rel{asset_db_->GuidToPath(*guid)};

        if (src_asset_dir_rel.empty()) {
          spdlog::error("Failed to get asset package path during drop. Ignoring.");
          DisplayError("Failed to move file.");
          return;
        }

        auto const src_abs{asset_db_->GetAssetDirectoryAbsolutePath() / src_asset_dir_rel};

        if (std::error_code ec; equivalent(src_abs.parent_path(), dir_target.path_abs, ec) || ec) {
          return;
        }

        ImGui::AcceptDragDropPayload(kAssetPackNodeDragDropTypeStr.data());

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
        auto const asset{ReflCast<NativeAsset*>(static_cast<ObjectDragDropPayload const*>(payload->Data)->ptr)};

        if (!asset || !asset_db_->IsAssetEditable(asset->GetAssetId())) {
          return;
        }

        auto const file_info{asset_db_->GetFileInfo(asset->GetAssetId().GetGuid())};

        if (!file_info) {
          spdlog::error("Failed to get file info for drop target asset. Ignoring.");
          DisplayError("Failed to move file.");
          return;
        }

        auto const src_abs{asset_db_->GetAssetDirectoryAbsolutePath() / file_info->src_path_asset_dir_rel};

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
    []([[maybe_unused]] NativeAssetFileProjectItem const& asset_item) {
      spdlog::error("Tried to drop onto a native asset file. Ignoring.");
    },
    []([[maybe_unused]] AssetPackageFileProjectItem const& asset_pack_item) {
      spdlog::error("Tried to drop onto an asset package file. Ignoring.");
    },
    []([[maybe_unused]] SubassetProjectItem const& subres_item) {
      spdlog::error("Tried to drop onto a subasset. Ignoring.");
    }
  }, item);

  ImGui::EndDragDropTarget();
}


auto ProjectWindow::CanRename(ProjectItem const& item) const -> bool {
  return std::visit(Overloaded{
    [this]([[maybe_unused]] DirectoryProjectItem const& dir_item) {
      // Don't allow renaming the root asset directory.
      return !IsAssetRootDirectory(dir_item);
    },
    []([[maybe_unused]] NativeAssetFileProjectItem const& asset_item) {
      return true;
    },
    []([[maybe_unused]] AssetPackageFileProjectItem const& asset_pack_item) {
      return true;
    },
    []([[maybe_unused]] SubassetProjectItem const& subres_item) {
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

      [&](NativeAssetFileProjectItem const& item) {
        return CommitAssetFileRename(item.guid, new_name);
      },

      [&](AssetPackageFileProjectItem const& item) {
        return CommitAssetFileRename(item.guid, new_name);
      },

      [this]([[maybe_unused]] SubassetProjectItem const& item) {
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
    rename_ctx_->error_msg = "A folder or asset with that name already exists.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  if (!asset_db_->MoveDirectory(src_path_abs, dst_path_abs)) {
    rename_ctx_->error_msg = "Failed to rename folder.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  selected_item_ = DirectoryProjectItem{
    .path_abs = dst_path_abs
  };

  return true;
}


auto ProjectWindow::CommitAssetFileRename(Guid const& guid, std::string_view const new_name) -> bool {
  auto info = asset_db_->GetFileInfo(guid);

  if (!info) {
    rename_ctx_->error_msg = "Asset file no longer exists.";
    return false;
  }

  auto const& old_path_asset_dir_rel{info->src_path_asset_dir_rel};

  auto const new_filename{
    std::string{new_name}.append(ToUntypedSv(old_path_asset_dir_rel.extension().u8string()))
  };

  auto const new_path_asset_dir_rel{old_path_asset_dir_rel.parent_path() / new_filename};

  if (old_path_asset_dir_rel == new_path_asset_dir_rel) {
    return true;
  }

  auto const dst_path_abs{
    asset_db_->GetAssetDirectoryAbsolutePath() / new_path_asset_dir_rel
  };

  if (std::filesystem::exists(dst_path_abs)) {
    rename_ctx_->error_msg = "A file or folder with that name already exists.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  if (!asset_db_->MoveAssetFile(guid, new_path_asset_dir_rel)) {
    rename_ctx_->error_msg = "Failed to rename asset file.";
    rename_ctx_->focus_requested = true;
    return false;
  }

  return true;
}


auto ProjectWindow::IsAssetRootDirectory(DirectoryProjectItem const& item) const -> bool {
  return IsAssetRootDirectory(item.path_abs);
}


auto ProjectWindow::IsAssetRootDirectory(std::filesystem::path const& path_abs) const -> bool {
  std::error_code ec;
  return exists(path_abs) && equivalent(path_abs, asset_db_->GetAssetDirectoryAbsolutePath(), ec) && !ec;
}


auto ProjectWindow::CanDelete(ProjectItem const& item) const -> bool {
  return std::visit(Overloaded{
    [this](DirectoryProjectItem const& dir_item) {
      return !IsAssetRootDirectory(dir_item);
    },
    []([[maybe_unused]] NativeAssetFileProjectItem const& native_asset_item) {
      return true;
    },
    []([[maybe_unused]] AssetPackageFileProjectItem const& asset_package_item) {
      return true;
    },
    []([[maybe_unused]] SubassetProjectItem const& subres_item) {
      return false;
    }
  }, item);
}


auto ProjectWindow::DrawImportSettingsDialog() -> void {
  auto constexpr popup_id{"Import Settings"};

  if (open_import_settings_dialog_) {
    open_import_settings_dialog_ = false;
    ImGui::OpenPopup(popup_id);
  }

  if (!ImGui::BeginPopupModal(popup_id, nullptr)) {
    return;
  }

  if (!import_settings_ctx_) {
    spdlog::error("Tried drawing import settings dialog but context is empty.");
    ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return;
  }

  auto& ctx{*import_settings_ctx_};

  ImGui::SeparatorText(ToUntypedSv(ctx.src_path_asset_dir_rel.filename().u8string()).data());

  auto changed_this_frame{false};
  drawer_registry_->Draw(*ctx.importer, true, changed_this_frame);
  ctx.dirty |= changed_this_frame;

  ImGui::Separator();

  ImGui::BeginDisabled(!ctx.dirty);

  if (ImGui::Button("Apply and reimport")) {
    if (asset_db_->ImportAssetFile(ctx.src_path_asset_dir_rel, ctx.importer.get())) {
      import_settings_ctx_.reset();
      ImGui::CloseCurrentPopup();
      ImGui::EndDisabled();
      ImGui::EndPopup();
      return;
    }

    auto constexpr err_msg{"Failed to reimport."};
    spdlog::error(err_msg);
    DisplayError(err_msg);
  }

  ImGui::EndDisabled();

  ImGui::SameLine();

  if (ImGui::Button("Cancel")) {
    import_settings_ctx_.reset();
    ImGui::CloseCurrentPopup();
  }

  ImGui::EndPopup();
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

    case ProjectCommandKind::kCopyAssetId: {
      ExecuteCopyAssetId(command->target);
      break;
    }

    case ProjectCommandKind::kUnloadAsset: {
      ExecuteUnloadAsset(command->target);
      break;
    }

    case ProjectCommandKind::kUnloadAllAssetsInFile: {
      ExecuteUnloadAllAssetsInFile(command->target);
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
      []([[maybe_unused]] AssetPackageFileProjectItem const& file) -> std::optional<std::filesystem::path> {
        spdlog::error("Import command received an asset package instead of a directory as target, ignoring.");
        return std::nullopt;
      },
      []([[maybe_unused]] NativeAssetFileProjectItem const& file) -> std::optional<std::filesystem::path> {
        spdlog::error("Import command received a native asset instead of a directory as target, ignoring.");
        return std::nullopt;
      },
      []([[maybe_unused]] SubassetProjectItem const& asset) -> std::optional<std::filesystem::path> {
        spdlog::error("Import command received a subasset instead of a directory as target, ignoring.");
        return std::nullopt;
      }
    }, target)
  };

  if (!target_dir_abs) {
    return;
  }

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
        std::format("Failed to import {}. File does not exist.", ToUntypedSv(src_path_abs.u8string()))
      };
      spdlog::error(err_msg);
      DisplayError(err_msg);
      return;
    }

    if (exists(dst_path_abs)) {
      auto const err_msg{
        std::format("Failed to import {}. File already exists.", ToUntypedSv(dst_path_abs.u8string()))
      };
      spdlog::error(err_msg);
      DisplayError(err_msg);
      return;
    }

    std::error_code ec;
    copy_file(src_path_abs, dst_path_abs, ec);

    if (ec) {
      auto const err_msg{
        std::format("Failed to import {}. Copy failed.", ToUntypedSv(src_path_abs.u8string()))
      };
      spdlog::error(err_msg);
      DisplayError(err_msg);
      return;
    }

    asset_db_->Refresh();
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
    auto const err_msg{std::format("Failed to create directory: {}.", ToUntypedSv(new_folder_path_abs.u8string()))};
    spdlog::error(err_msg);
    DisplayError(err_msg);
    return;
  }

  asset_db_->Refresh();
}


auto ProjectWindow::ExecuteCreateMaterial(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    [this](DirectoryProjectItem const& item) {
      auto mtl{std::make_unique<Material>()};
      auto const mtl_path_abs{GenerateUniquePath(item.path_abs / "New Material.mtl")};

      auto created_mtl{app_->GetAssetDatabase().SaveAssetToFile(std::move(mtl), mtl_path_abs)};

      if (!created_mtl) {
        auto const err_msg{
          std::format("Failed to create new material at {}.", ToUntypedSv(mtl_path_abs.u8string()))
        };
        spdlog::error(err_msg);
        DisplayError(err_msg);
        return;
      }

      SelectItem(NativeAssetFileProjectItem{
        created_mtl->GetAssetId().GetGuid()
      });
    },
    []([[maybe_unused]] NativeAssetFileProjectItem const& item) {
      spdlog::error("Tried to create material at a native asset file target. Ignoring.");
    },
    []([[maybe_unused]] AssetPackageFileProjectItem const& item) {
      spdlog::error("Tried to create material at an asset package file target. Ignoring.");
    },
    []([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried to create material and a subasset target. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteCreateScene(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    [this](DirectoryProjectItem const& item) {
      auto scene{std::make_unique<Scene>()};
      auto const scene_path_abs{GenerateUniquePath(item.path_abs / "New Scene.scene")};

      auto created_scene{app_->GetAssetDatabase().SaveAssetToFile(std::move(scene), scene_path_abs)};

      if (!created_scene) {
        auto const err_msg{
          std::format("Failed to create new scene at {}.", ToUntypedSv(scene_path_abs.u8string()))
        };
        spdlog::error(err_msg);
        DisplayError(err_msg);
        return;
      }

      SelectItem(NativeAssetFileProjectItem{
        created_scene->GetAssetId().GetGuid()
      });
    },
    []([[maybe_unused]] NativeAssetFileProjectItem const& item) {
      spdlog::error("Tried to create scene at a native asset file target. Ignoring.");
    },
    []([[maybe_unused]] AssetPackageFileProjectItem const& item) {
      spdlog::error("Tried to create scene at an asset package file target. Ignoring.");
    },
    []([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried to create scene and a subasset target. Ignoring.");
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

      [this](NativeAssetFileProjectItem const& item) -> std::optional<std::string> {
        auto const info{asset_db_->GetFileInfo(item.guid)};
        if (!info) {
          return std::nullopt;
        }
        return info->src_path_asset_dir_rel.stem().string();
      },

      [this](AssetPackageFileProjectItem const& item) -> std::optional<std::string> {
        auto const info{asset_db_->GetFileInfo(item.guid)};
        if (!info) {
          return std::nullopt;
        }
        return info->src_path_asset_dir_rel.stem().string();
      },

      [](SubassetProjectItem const&) -> std::optional<std::string> {
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
    [this](NativeAssetFileProjectItem const& item) {
      ExecuteDeleteAssetFile(item.guid);
    },
    [this](AssetPackageFileProjectItem const& item) {
      ExecuteDeleteAssetFile(item.guid);
    },
    []([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried deleting a subasset. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteDeleteDirectory(DirectoryProjectItem const& target) -> void {
  if (IsAssetRootDirectory(target)) {
    auto constexpr err_msg{"Tried deleting asset root directory. Ignoring."};
    spdlog::error(err_msg);
    DisplayError(err_msg);
    return;
  }

  if (!app_->GetAssetDatabase().DeleteDirectory(target.path_abs)) {
    auto const err_msg{std::format("Failed to delete folder: {}", ToUntypedSv(target.path_abs.u8string()))};
    spdlog::error(err_msg);
    DisplayError(err_msg);
    return;
  }

  ClearSelection();
}


auto ProjectWindow::ExecuteDeleteAssetFile(Guid const& target) -> void {
  if (!app_->GetAssetDatabase().GetFileInfo(target)) {
    spdlog::error("Tried deleting asset file with GUID {} that does not exist. Ignoring.", target.ToString());
    return;
  }

  app_->GetAssetDatabase().DeleteAssetFile(target);
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
    [this](NativeAssetFileProjectItem const& item) {
      auto const path_asset_dir_rel{asset_db_->GuidToPath(item.guid)};
      if (path_asset_dir_rel.empty()) {
        spdlog::error("Tried showing a native asset file in explorer, but failed to get its path.");
        DisplayError("Failed to get file path.");
        return;
      }
      if (!ShowInFileExplorer(asset_db_->GetAssetDirectoryAbsolutePath() / path_asset_dir_rel)) {
        spdlog::error("Failed to show native asset file in explorer.");
        DisplayError("Failed to open File Explorer.");
      }
    },
    [this](AssetPackageFileProjectItem const& item) {
      auto const path_asset_dir_rel{asset_db_->GuidToPath(item.guid)};
      if (path_asset_dir_rel.empty()) {
        spdlog::error("Tried showing an asset package file in explorer, but failed to get its path.");
        DisplayError("Failed to get file path.");
        return;
      }
      if (!ShowInFileExplorer(asset_db_->GetAssetDirectoryAbsolutePath() / path_asset_dir_rel)) {
        spdlog::error("Failed to show asset package file in explorer.");
        DisplayError("Failed to open File Explorer.");
      }
    },
    []([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried showing a subasset in explorer. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteReimport(ProjectItem const& target) const -> void {
  std::visit(Overloaded{
    []([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried reimporting a folder. Ignoring.");
    },
    []([[maybe_unused]] NativeAssetFileProjectItem const& item) {
      spdlog::error("Tried reimporting a native asset. Ignoring.");
    },
    [this](AssetPackageFileProjectItem const& item) {
      auto const file_info{asset_db_->GetFileInfo(item.guid)};

      if (!file_info) {
        spdlog::error("Tried to reimport but failed to get file info.");
        DisplayError("Failed to reimport.");
        return;
      }

      auto const src_path_abs{asset_db_->GetAssetDirectoryAbsolutePath() / file_info->src_path_asset_dir_rel};
      auto const importer{AssetDB::GetImporterForAssetFile(src_path_abs)};

      if (!importer) {
        spdlog::error("Tried to reimport but failed to get importer.");
        DisplayError("Failed to reimport.");
        return;
      }

      if (!asset_db_->ImportAssetFile(file_info->src_path_asset_dir_rel, importer.get())) {
        auto constexpr err_msg{"Failed to reimport."};
        spdlog::error(err_msg);
        DisplayError(err_msg);
      }
    },
    []([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried reimporting a subasset. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::OpenImportSettings(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    []([[maybe_unused]] DirectoryProjectItem const&) {
      spdlog::error("Tried opening import settings for a folder. Ignoring.");
    },

    []([[maybe_unused]] NativeAssetFileProjectItem const&) {
      spdlog::error("Tried opening import settings for a native asset. Ignoring.");
    },

    [this](AssetPackageFileProjectItem const& item) {
      auto const file_info{asset_db_->GetFileInfo(item.guid)};

      if (!file_info) {
        spdlog::error("Tried opening import settings but failed to get file info.");
        DisplayError("Failed to open import settings.");
        return;
      }

      auto const src_path_abs{asset_db_->GetAssetDirectoryAbsolutePath() / file_info->src_path_asset_dir_rel};
      auto importer{AssetDB::GetImporterForAssetFile(src_path_abs)};

      if (!importer) {
        spdlog::error("Tried opening import settings but failed to get importer.");
        DisplayError("Failed to open import settings.");
        return;
      }

      import_settings_ctx_ = ImportSettingsContext{
        .src_path_asset_dir_rel = file_info->src_path_asset_dir_rel,
        .importer = std::move(importer),
        .dirty = false
      };

      open_import_settings_dialog_ = true;
    },

    []([[maybe_unused]] SubassetProjectItem const&) {
      spdlog::error("Tried opening import settings for a subasset. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteCopyGuid(ProjectItem const& target) const -> void {
  std::visit(Overloaded{
    []([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried copying GUID of a directory. Ignoring.");
    },
    [this](NativeAssetFileProjectItem const& item) {
      if (!CopyToClipboard(item.guid.ToString())) {
        spdlog::error("Failed to copy GUID to clipboard.");
      }
    },
    [this](AssetPackageFileProjectItem const& item) {
      if (!CopyToClipboard(item.guid.ToString())) {
        spdlog::error("Failed to copy GUID to clipboard.");
      }
    },
    []([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried copying GUID of a subasset. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteCopyAssetId(ProjectItem const& target) const -> void {
  std::visit(Overloaded{
    [this]([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried copying asset ID of a directory. Ignoring.");
    },
    [this](NativeAssetFileProjectItem const& item) {
      if (!CopyToClipboard(AssetId{item.guid, 0}.ToString())) {
        spdlog::error("Failed to copy asset ID to clipboard.");
      }
    },
    [this]([[maybe_unused]] AssetPackageFileProjectItem const& item) {
      spdlog::error("Tried copying asset ID of an asset package file. Ignoring.");
    },
    [this](SubassetProjectItem const& item) {
      if (!CopyToClipboard(item.id.ToString())) {
        spdlog::error("Failed to copy asset ID to clipboard.");
      }
    }
  }, target);
}


auto ProjectWindow::ExecuteUnloadAsset(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    []([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried unloading a directory. Ignoring.");
    },
    [this](NativeAssetFileProjectItem const& item) {
      ExecuteUnloadAssetById(AssetId{item.guid, 0});
    },
    [this]([[maybe_unused]] AssetPackageFileProjectItem const& item) {
      spdlog::error("Tried unloading an asset package file. Ignoring.");
    },
    [this](SubassetProjectItem const& item) {
      ExecuteUnloadAssetById(item.id);
    }
  }, target);
}


auto ProjectWindow::ExecuteUnloadAllAssetsInFile(ProjectItem const& target) -> void {
  std::visit(Overloaded{
    [this]([[maybe_unused]] DirectoryProjectItem const& item) {
      spdlog::error("Tried unloading all assets of a directory. Ignoring.");
    },
    [this]([[maybe_unused]] NativeAssetFileProjectItem const& item) {
      spdlog::error("Tried unloading all assets of a native asset file. Ignoring.");
    },
    [this](AssetPackageFileProjectItem const& item) {
      std::vector<ObserverPtr<AssetDB::AssetInfo const>> subassets;
      asset_db_->GetAssetsInFile(item.guid, subassets);
      for (auto const& subasset : subassets) {
        ExecuteUnloadAssetById(subasset->id);
      }
    },
    [this]([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried unloading all assets in a subasset. Ignoring.");
    }
  }, target);
}


auto ProjectWindow::ExecuteUnloadAssetById(AssetId const& target) -> void {
  App::Instance().GetAssetManager().Unload(target);
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

        if (!asset_db_->MoveDirectory(move_to_folder_ctx->src_abs, dst_abs)) {
          spdlog::error("Failed to move directory {} to {}.", ToUntypedSv(move_to_folder_ctx->src_abs.u8string()),
            ToUntypedSv(dst_abs.u8string()));
          DisplayError("Failed to move folder.");
        }
      } else {
        if (!IsSubpath(move_to_folder_ctx->src_abs, asset_db_->GetAssetDirectoryAbsolutePath())) {
          spdlog::error("Tried moving a file that is not in asset directory. Ignoring.");
          return;
        }

        auto const src_asset_dir_rel{
          relative(move_to_folder_ctx->src_abs, asset_db_->GetAssetDirectoryAbsolutePath())
        };

        auto const dst_asset_dir_rel{
          relative(dst_abs, asset_db_->GetAssetDirectoryAbsolutePath())
        };

        auto const guid{asset_db_->PathToGuid(src_asset_dir_rel)};

        if (!guid.IsValid()) {
          spdlog::error("Tried moving a file that is not in asset database. Ignoring.");
          return;
        }

        if (!asset_db_->MoveAssetFile(guid, dst_asset_dir_rel)) {
          spdlog::error("Failed to move asset file {} to {}.",
            ToUntypedSv(move_to_folder_ctx->src_abs.u8string()),
            ToUntypedSv(dst_abs.u8string()));
          DisplayError("Failed to move asset file.");
        }
      }
    },
    []([[maybe_unused]] NativeAssetFileProjectItem const& item) {
      spdlog::error("Tried moving into a native asset file. Ignoring.");
    },
    []([[maybe_unused]] AssetPackageFileProjectItem const& item) {
      spdlog::error("Tried moving into an asset package file. Ignoring.");
    },
    []([[maybe_unused]] SubassetProjectItem const& item) {
      spdlog::error("Tried moving into a subasset. Ignoring.");
    }
  }, target);
}
}
