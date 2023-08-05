#include "ProjectWindow.hpp"

#include "Material.hpp"
#include "Util.hpp"
#include "Platform.hpp"
#include "ReflectionDisplayProperties.hpp"

#include <nfd.h>
#include <imgui_stdlib.h>

#include <optional>


namespace sorcery::mage {
auto ProjectWindow::DrawFilesystemTree(std::filesystem::path const& resDirAbs, std::filesystem::path const& thisPathResDirRel) noexcept -> bool {
  auto ret{false};
  auto thisPathAbs{canonical(resDirAbs / thisPathResDirRel)};
  auto selectedPathAbs{resDirAbs / mSelectedPathResDirRel};
  auto const isSelected{exists(thisPathAbs) && exists(selectedPathAbs) && equivalent(thisPathAbs, selectedPathAbs)};
  auto const isRenaming{mRenameInfo && exists(mRenameInfo->nodePathAbs) && exists(thisPathAbs) && equivalent(mRenameInfo->nodePathAbs, thisPathAbs)};
  auto const isDirectory{is_directory(thisPathAbs)};

  ImGuiTreeNodeFlags treeNodeFlags{ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick};

  if (!isDirectory) {
    treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
  } else {
    treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
  }

  if (isSelected) {
    treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
  }

  if (isRenaming) {
    treeNodeFlags |= ImGuiTreeNodeFlags_AllowOverlap;
    ImGui::SetNextItemAllowOverlap();
  }

  auto const treeNodePos{ImGui::GetCursorPos()};

  if (ImGui::TreeNodeEx(std::format("{}{}", isRenaming ? "##" : "", thisPathAbs.stem().string()).c_str(), treeNodeFlags)) {
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      mSelectedPathResDirRel = thisPathResDirRel;
      selectedPathAbs = resDirAbs / mSelectedPathResDirRel;
      mApp->SetSelectedObject(gResourceManager.Load(mApp->GetResourceDatabase().PathToGuid(thisPathResDirRel)));
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      mOpenContextMenu = true;
    }

    if (isRenaming) {
      ImGui::SetKeyboardFocusHere();
      ImGui::SetCursorPos(treeNodePos);

      if (ImGui::InputText("##Rename", &mRenameInfo->newName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
        auto const newPathAbs{mRenameInfo->nodePathAbs.parent_path() / mRenameInfo->newName += mRenameInfo->nodePathAbs.extension()};
        auto const newPathResDirRel{newPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())};

        if (isDirectory
              ? mApp->GetResourceDatabase().MoveDirectory(mRenameInfo->nodePathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath()), newPathResDirRel)
              : mApp->GetResourceDatabase().MoveResource(mApp->GetResourceDatabase().PathToGuid(thisPathResDirRel), newPathResDirRel)) {
          mSelectedPathResDirRel = newPathAbs.lexically_relative(resDirAbs);
          selectedPathAbs = resDirAbs / mSelectedPathResDirRel;
          thisPathAbs = newPathAbs;
          ret = true;
        }

        mRenameInfo.reset();
      }

      if ((!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        mRenameInfo.reset();
      }
    } else if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
      mSelectedPathResDirRel = thisPathResDirRel;
      selectedPathAbs = resDirAbs / mSelectedPathResDirRel;
      StartRenamingSelected();
    }

    if (isDirectory) {
      for (auto const& entry : std::filesystem::directory_iterator{thisPathAbs}) {
        if (entry.path().extension() != ResourceManager::RESOURCE_META_FILE_EXT) {
          if (DrawFilesystemTree(resDirAbs, relative(entry.path(), resDirAbs))) {
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


auto ProjectWindow::OpenFileDialog(std::string_view const filters, std::string_view const defaultPath, std::filesystem::path& out) -> bool {
  if (nfdchar_t* selectedPath{nullptr}; NFD_OpenDialog(filters.data(), defaultPath.data(), &selectedPath) == NFD_OKAY) {
    out = selectedPath;
    std::free(selectedPath);
    return true;
  }

  return false;
}


auto ProjectWindow::DrawContextMenu() noexcept -> void {
  if (ImGui::BeginPopup(CONTEXT_MENU_ID.data())) {
    auto const selectedPathAbs{weakly_canonical(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel)};
    auto const workingDirAbs{is_directory(selectedPathAbs) ? selectedPathAbs : selectedPathAbs.parent_path()};
    auto const isResDirSelected{equivalent(selectedPathAbs, mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())};

    if (ImGui::BeginMenu("New")) {
      if (ImGui::MenuItem("Folder")) {
        auto const newFolderPathAbs{GenerateUniquePath(workingDirAbs / "New Folder")};
        create_directory(newFolderPathAbs);

        mSelectedPathResDirRel = newFolderPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
        mApp->SetSelectedObject(nullptr);
      }

      if (ImGui::MenuItem("Material")) {
        auto const mtl{new Material{}};
        auto const mtlPathAbs{GenerateUniquePath(workingDirAbs / "New Material.mtl")};
        mApp->GetResourceDatabase().CreateResource(*mtl, mtlPathAbs);
        mSelectedPathResDirRel = mtlPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
        mApp->SetSelectedObject(mtl);
      }

      if (ImGui::MenuItem("Scene")) {
        auto const scene{new Scene{}};
        auto const scenePathAbs{GenerateUniquePath(workingDirAbs / "New Scene.scene")};
        mApp->GetResourceDatabase().CreateResource(*scene, scenePathAbs);
        mApp->SetSelectedObject(scene);
      }

      ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Import")) {
      if (std::filesystem::path srcPathAbs; OpenFileDialog("", "", srcPathAbs) && !IsSubpath(srcPathAbs, mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())) {
        auto const dstPathAbs{workingDirAbs / srcPathAbs.filename()};
        copy_file(srcPathAbs, dstPathAbs);
        auto const dstPathResDirRel{dstPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())};

        if (auto const importerNode{mApp->GetResourceDatabase().FindImporterForResourceFile(dstPathResDirRel)}; !importerNode.IsNull()) {
          mImportModal.node = importerNode;
          mImportModal.dstPathResDirRel = dstPathResDirRel;
          mOpenImportModal = true;
        }
      }
    }

    if (ImGui::MenuItem("Rename", nullptr, nullptr, !isResDirSelected)) {
      StartRenamingSelected();
    }

    if (ImGui::MenuItem("Delete", nullptr, nullptr, !is_directory(selectedPathAbs))) {
      if (is_directory(selectedPathAbs)) {
        // TODO delete resources in folder
      } else {
        mApp->SetSelectedObject(nullptr);
        mApp->GetResourceDatabase().DeleteResource(mApp->GetResourceDatabase().PathToGuid(mSelectedPathResDirRel));
      }
    }

    ImGui::EndPopup();
  }
}


auto ProjectWindow::StartRenamingSelected() noexcept -> void {
  mRenameInfo = RenameInfo{.newName = mSelectedPathResDirRel.stem().string(), .nodePathAbs = mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel};
}


ProjectWindow::ProjectWindow(Application& context) :
  mApp{&context} { }


auto ProjectWindow::Draw() -> void {
  if (ImGui::Begin("Project", &mIsOpen, ImGuiWindowFlags_NoCollapse)) {
    auto const clickedOutside{!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows) && ImGui::IsAnyItemHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))};
    auto const selectedDoesNotExist{!mSelectedPathResDirRel.empty() && !exists(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel)};
    if (clickedOutside || selectedDoesNotExist) {
      mSelectedPathResDirRel.clear();
    }

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      mOpenContextMenu = true;
    }

    std::ignore = DrawFilesystemTree(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath(), {});

    if (mOpenContextMenu) {
      mOpenContextMenu = false;
      ImGui::OpenPopup(CONTEXT_MENU_ID.data());
    }

    DrawContextMenu();

    auto constexpr importModalId{"Importer Settings"};

    if (mOpenImportModal) {
      mOpenImportModal = false;
      ImGui::OpenPopup(importModalId);
    }

    if (ImGui::BeginPopupModal(importModalId)) {
      ReflectionDisplayProperties(mImportModal.node);

      if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
      }

      ImGui::SameLine();

      if (ImGui::Button("Import")) {
        mApp->GetResourceDatabase().ImportResource(mImportModal.dstPathResDirRel);
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }
  }
  ImGui::End();
}
}
