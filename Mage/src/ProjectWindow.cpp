#include "ProjectWindow.hpp"

#include <optional>
#include <queue>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <nfd.hpp>
#include <spdlog/spdlog.h>

#include "EditorApp.hpp"
#include "GUI.hpp"
#include "Material.hpp"
#include "ReflectionDisplayProperties.hpp"
#include "util.hpp"


namespace sorcery::mage {
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
        root_node_->children.emplace_back(ProjectTreeNode{
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
          root_node_->children.emplace_back(ProjectTreeNode{
            .item = NativeResourceFileProjectItem{.guid = guid},
            .display_name = entry.path().filename().string(),
            .imgui_id = std::format("##{}", entry.path().string()),
            .children = {}
          });
        } else {
          std::vector<ObserverPtr<ResourceDB::ResourceInfo const>> subresources;
          resource_db_->GetResourcesInFile(guid, subresources);

          if (subresources.empty()) {
            continue;
          }

          std::vector<ProjectTreeNode> subresource_nodes;

          for (auto const& subresource : subresources) {
            subresource_nodes.emplace_back(ProjectTreeNode{
              .item = SubresourceProjectItem{.id = subresource->id},
              .display_name = subresource->name,
              .imgui_id = std::format("##{}", std::string{subresource->id}),
              .children = {}
            });
          }

          root_node_->children.emplace_back(ProjectTreeNode{
            .item = ResourcePackageFileProjectItem{.guid = guid},
            .display_name = entry.path().filename().string(),
            .imgui_id = std::format("##{}", entry.path().string()),
            .children = std::move(subresource_nodes)
          });
        }
      }
    }
  }

  spdlog::debug("Finished project window hierarchy rebuild.");
}


auto ProjectWindow::Draw() -> void {
  if (should_rebuild_hierarchy_on_next_draw_) {
    RebuildHierarchy();
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

  ImGuiTreeNodeFlags flags =
    ImGuiTreeNodeFlags_OpenOnArrow |
    ImGuiTreeNodeFlags_SpanAvailWidth;

  if (!draw_as_tree) {
    flags |= ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen;
  }

  if (selected_item_ && *selected_item_ == node.item) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  auto const is_open{ImGui::TreeNodeEx(node.imgui_id.c_str(), flags, "%s", node.display_name.c_str())};

  if (ImGui::IsItemClicked()) {
    SelectItem(node.item);
  }

  // TODO Draw context menu here

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


auto ProjectWindow::DrawFilesystemTree(std::filesystem::path const& node_path_abs,
                                       std::filesystem::path const& node_path_res_dir_rel,
                                       bool const is_directory) noexcept -> bool {
  auto ret{false};

  auto& res_db{app_->GetResourceDatabase()};
  auto const& res_dir_abs{res_db.GetResourceDirectoryAbsolutePath()};

  auto this_path_res_dir_rel{node_path_res_dir_rel};
  auto this_path_abs{node_path_abs};
  auto selected_path_abs{mSelectedPathResDirRel.empty() ? res_dir_abs : res_dir_abs / mSelectedPathResDirRel};

  auto const is_selected{this_path_abs == selected_path_abs};
  auto const is_renaming{rename_ctx_ && rename_ctx_->node_path_abs == this_path_abs};

  ImGuiTreeNodeFlags tree_node_flags{ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick};

  if (!is_directory) {
    tree_node_flags |= ImGuiTreeNodeFlags_Leaf;
  } else {
    tree_node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
  }

  if (is_selected) {
    tree_node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  if (is_renaming) {
    tree_node_flags |= ImGuiTreeNodeFlags_AllowOverlap;
    ImGui::SetNextItemAllowOverlap();
  }

  auto const tree_node_pos{ImGui::GetCursorPos()};
  auto tree_node_label{this_path_abs.stem().string()};
  auto const node_is_open{
    ImGui::TreeNodeEx((is_renaming ? tree_node_label.insert(0, "##") : tree_node_label).c_str(), tree_node_flags)
  };

  if (is_renaming) {
    ImGui::SetKeyboardFocusHere();
    ImGui::SetCursorPos(tree_node_pos);

    if (ImGui::InputText("##Rename", &rename_ctx_->new_name,
      ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
      auto const newPathAbs{
        rename_ctx_->node_path_abs.parent_path() / rename_ctx_->new_name += rename_ctx_->node_path_abs.extension()
      };
      auto const newPathResDirRel{relative(newPathAbs, res_dir_abs)};

      if (is_directory
            ? res_db.MoveDirectory(relative(rename_ctx_->node_path_abs, res_dir_abs), newPathResDirRel)
            : res_db.MoveResourceFile(res_db.PathToGuid(this_path_res_dir_rel), newPathResDirRel)) {
        this_path_abs = newPathAbs;
        this_path_res_dir_rel = relative(this_path_abs, res_dir_abs);
        mSelectedPathResDirRel = this_path_res_dir_rel;
        selected_path_abs = res_dir_abs / mSelectedPathResDirRel;
        ret = true;
      }

      rename_ctx_.reset();
    }

    if ((!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || ImGui::IsKeyPressed(
          ImGuiKey_Escape)) {
      rename_ctx_.reset();
    }
  } else if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (
               is_selected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
    mSelectedPathResDirRel = this_path_res_dir_rel;
    selected_path_abs = res_dir_abs / mSelectedPathResDirRel;
    StartRenamingSelected();
  }

  if (!this_path_res_dir_rel.empty() && ImGui::BeginDragDropSource()) {
    if (is_directory) {
      auto const thisPathResDirRelStr{this_path_res_dir_rel.string()};
      ImGui::SetDragDropPayload(kDirNodeDragDropTypeStr.data(), thisPathResDirRelStr.c_str(),
        thisPathResDirRelStr.size() + 1);
    } else {
      auto const res{
        App::Instance().GetResourceManager().GetOrLoad(ResourceId{res_db.PathToGuid(this_path_res_dir_rel), 0})
      };
      ImGui::SetDragDropPayload(ObjectDragDropData::TYPE_STR.data(), &res, sizeof(decltype(res)));
    }
    ImGui::EndDragDropSource();
  }

  if (is_directory && ImGui::BeginDragDropTarget()) {
    if (auto const payload{ImGui::AcceptDragDropPayload(kDirNodeDragDropTypeStr.data())}) {
      std::string payloadPathResDirRelStr(static_cast<std::size_t>(payload->DataSize), '\0');
      std::memcpy(payloadPathResDirRelStr.data(), payload->Data, payload->DataSize);
      std::filesystem::path const payloadPathResDirRel{payloadPathResDirRelStr};

      if (res_db.MoveDirectory(payloadPathResDirRel, this_path_res_dir_rel / payloadPathResDirRel.filename())) {
        ret = true;
      }
    }

    if (auto const payload{ImGui::AcceptDragDropPayload(ObjectDragDropData::TYPE_STR.data())}) {
      if (auto const objectDragDropData{static_cast<ObjectDragDropData*>(payload->Data)};
        objectDragDropData && objectDragDropData->ptr && rttr::type::get(*objectDragDropData->ptr).is_derived_from(
          rttr::type::get<Resource>())) {
        if (auto const res{static_cast<Resource*>(objectDragDropData->ptr)}; res_db.MoveResourceFile(
          res->GetId().GetGuid(),
          this_path_res_dir_rel / res_db.GuidToPath(res->GetId().GetGuid()).filename())) {
          ret = true;
        }
      }
    }

    ImGui::EndDragDropTarget();
  }

  if ((ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) || ImGui::IsItemClicked(
        ImGuiMouseButton_Right)) {
    mSelectedPathResDirRel = this_path_res_dir_rel;
    selected_path_abs = res_dir_abs / mSelectedPathResDirRel;
    app_->SetSelectedObject(App::Instance().GetResourceManager().GetOrLoad(ResourceId{
      res_db.PathToGuid(this_path_res_dir_rel), 0
    }));
  }

  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    open_context_menu_ = true;
  }

  if (node_is_open) {
    if (is_directory) {
      for (auto const& entry : std::filesystem::directory_iterator{this_path_abs}) {
        if (entry.path().extension() != ResourceDB::kResourceMetaFileExt) {
          if (DrawFilesystemTree(entry.path(), this_path_res_dir_rel / entry.path().filename(), entry.is_directory())) {
            // The directory_iterator does not guarantee anything when the directory tree changes, it's safer to skip the rest of the frame.
            break;
          }
        }
      }
    }

    ImGui::TreePop();
  }

  return ret;
}


auto ProjectWindow::DrawContextMenu() -> void {
  if (ImGui::BeginPopup(kContextMenuId.data())) {
    auto const selectedPathAbs{
      weakly_canonical(app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel)
    };
    auto const workingDirAbs{is_directory(selectedPathAbs) ? selectedPathAbs : selectedPathAbs.parent_path()};
    auto const isResDirSelected{
      exists(selectedPathAbs) && exists(app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath()) && equivalent(
        selectedPathAbs, app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath())
    };

    if (ImGui::BeginMenu("New")) {
      if (ImGui::MenuItem("Folder")) {
        auto const newFolderPathAbs{GenerateUniquePath(workingDirAbs / "New Folder")};
        create_directory(newFolderPathAbs);

        mSelectedPathResDirRel = relative(newFolderPathAbs,
          app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
        app_->SetSelectedObject(nullptr);
      }

      if (ImGui::MenuItem("Material")) {
        auto mtl{Create<Material>()};
        auto const mtlPathAbs{GenerateUniquePath(workingDirAbs / "New Material.mtl")};
        auto const selection{app_->GetResourceDatabase().SaveResourceToFile(std::move(mtl), mtlPathAbs)};
        mSelectedPathResDirRel = relative(mtlPathAbs, app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
        app_->SetSelectedObject(selection.Get());
      }

      if (ImGui::MenuItem("Scene")) {
        auto scene{Create<Scene>()};
        auto const scenePathAbs{GenerateUniquePath(workingDirAbs / "New Scene.scene")};
        auto const selection{app_->GetResourceDatabase().SaveResourceToFile(std::move(scene), scenePathAbs)};
        app_->SetSelectedObject(selection.Get());
      }

      ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Import")) {
      if (NFD::UniquePathSet dst_paths;
        OpenDialogMultiple(dst_paths, static_cast<nfdnfilteritem_t*>(nullptr)) == NFD_OKAY) {
        if (nfdpathsetsize_t path_set_size{0}; NFD::PathSet::Count(dst_paths, path_set_size) == NFD_OKAY) {
          for (nfdpathsetsize_t i{0}; i < path_set_size; i++) {
            if (NFD::UniquePathSetPathN src_path_abs_str;
              NFD::PathSet::GetPath(dst_paths, i, src_path_abs_str) == NFD_OKAY) {
              std::filesystem::path const src_path_abs{src_path_abs_str.get()};

              if (auto importer{ResourceDB::CreateNewImporterForResourceFile(src_path_abs)}) {
                if (auto const dst_path_abs{GenerateUniquePath(workingDirAbs / src_path_abs.filename())};
                  !TryImportFromSourceFile(importer.get(), src_path_abs, dst_path_abs)) {
                  ImGui::CloseCurrentPopup();
                  ImGui::EndPopup();
                  ImGui::End();
                  throw std::runtime_error{std::format("Failed to import {}.", dst_path_abs.string())};
                }
              } else {
                ImGui::EndPopup();
                throw std::runtime_error{
                  std::format("Couldn't find importer for file type {}.", src_path_abs.extension().string())
                };
              }
            }
          }
        }
      }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Import Settings", nullptr, nullptr, !is_directory(selectedPathAbs))) {
      if (std::unique_ptr importer{ResourceDB::GetImporterForResourceFile(selectedPathAbs)}) {
        files_to_import_.emplace_back(std::move(importer), selectedPathAbs, selectedPathAbs);
        open_import_modal_ = true;
      }
    }

    if (ImGui::MenuItem("Rename", nullptr, nullptr, !isResDirSelected)) {
      StartRenamingSelected();
    }

    if (ImGui::MenuItem("Delete", nullptr, nullptr, !isResDirSelected)) {
      if (is_directory(selectedPathAbs)) {
        if (app_->GetResourceDatabase().DeleteDirectory(selectedPathAbs)) {
          app_->SetSelectedObject(nullptr);
          mSelectedPathResDirRel.clear();
        }
      } else {
        app_->GetResourceDatabase().DeleteResourceFile(app_->GetResourceDatabase().PathToGuid(mSelectedPathResDirRel));
        app_->SetSelectedObject(nullptr);
        mSelectedPathResDirRel.clear();
      }
    }

    ImGui::EndPopup();
  }
}


auto ProjectWindow::StartRenamingSelected() noexcept -> void {
  rename_ctx_ = RenameContext{
    .new_name = mSelectedPathResDirRel.stem().string(),
    .node_path_abs = app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel
  };
}


auto ProjectWindow::TryImportFromSourceFile(ResourceImporter* const importer, std::filesystem::path const& src_path_abs,
                                            std::filesystem::path const& dst_path_abs) const -> bool {
  if (exists(src_path_abs) && !exists(dst_path_abs)) {
    copy_file(src_path_abs, dst_path_abs);
  }

  if (!app_->GetResourceDatabase().ImportResourceFile(
    relative(dst_path_abs, app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath()), importer)) {
    remove(dst_path_abs);
    return false;
  }

  return true;
}
}
