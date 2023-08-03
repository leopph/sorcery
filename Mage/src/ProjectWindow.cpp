// ReSharper disable All
#define _CRT_SECURE_NO_WARNINGS
// ReSharper restore All
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ProjectWindow.hpp"

#include "Material.hpp"
#include "Util.hpp"
#include "Platform.hpp"
#include "ReflectionDisplayProperties.hpp"

#include <nfd.h>
#include <imgui_stdlib.h>

#include <optional>


namespace sorcery::mage {
auto ProjectWindow::DrawFilesystemTree(std::filesystem::path const& resDirAbs, std::filesystem::path const& thisPathResDirRel) -> void {
  auto pathAbs{canonical(resDirAbs / thisPathResDirRel)};
  auto const isSelected{equivalent(pathAbs, resDirAbs / mSelectedPathResDirRel)};
  auto const isRenaming{mRenameInfo && equivalent(mRenameInfo->nodePathAbs, pathAbs)};
  auto const isDirectory{is_directory(pathAbs)};

  ImGuiTreeNodeFlags treeNodeFlags{ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick};

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
  } else {
    treeNodeFlags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
  }

  auto const treeNodePos{ImGui::GetCursorPos()};

  if (ImGui::TreeNodeEx(std::format("{}{}", isRenaming
                                              ? "##"
                                              : "", pathAbs.stem().string()).c_str(), treeNodeFlags)) {
    if (ImGui::IsItemClicked()) {
      mSelectedPathResDirRel = thisPathResDirRel;
      mApp->SetSelectedObject(gResourceManager.Load(mApp->GetResourceDatabase().PathToGuid(thisPathResDirRel)));
    }

    auto const startRenaming{
      [this, &pathAbs] {
        mRenameInfo = RenameInfo{.newName = pathAbs.stem().string(), .nodePathAbs = pathAbs};
      }
    };

    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::MenuItem("Rename")) {
        startRenaming();
      }

      if (ImGui::MenuItem("Delete")) {
        mApp->GetResourceDatabase().DeleteResource(mApp->GetResourceDatabase().PathToGuid(thisPathResDirRel));
      }

      ImGui::EndPopup();
    }

    if (isRenaming) {
      ImGui::SetKeyboardFocusHere();
      ImGui::SetCursorPos(treeNodePos);

      if (ImGui::InputText("##Rename", &mRenameInfo->newName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
        auto const newPathAbs{mRenameInfo->nodePathAbs.parent_path() / mRenameInfo->newName += mRenameInfo->nodePathAbs.extension()};

        if (isDirectory) {
          std::filesystem::rename(mRenameInfo->nodePathAbs, newPathAbs);
        } else {
          mApp->GetResourceDatabase().MoveResource(mApp->GetResourceDatabase().PathToGuid(thisPathResDirRel), newPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath()));
        }

        mRenameInfo.reset();

        if (isSelected) {
          mSelectedPathResDirRel = relative(newPathAbs, resDirAbs);
        }

        pathAbs = newPathAbs;
      }

      if ((!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        mRenameInfo.reset();
      }
    } else if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
      startRenaming();
    }

    if (isDirectory) {
      for (auto const& entry : std::filesystem::directory_iterator{pathAbs}) {
        if (entry.path().extension() != ResourceManager::RESOURCE_META_FILE_EXT) {
          DrawFilesystemTree(resDirAbs, relative(entry.path(), resDirAbs));
        }
      }
    }

    ImGui::TreePop();
  }
}


auto ProjectWindow::OpenFileDialog(std::string_view const filters, std::string_view const defaultPath, std::filesystem::path& out) -> bool {
  if (nfdchar_t* selectedPath{nullptr}; NFD_OpenDialog(filters.data(), defaultPath.data(), &selectedPath) == NFD_OKAY) {
    out = selectedPath;
    std::free(selectedPath);
    return true;
  }

  return false;
}


ProjectWindow::ProjectWindow(Application& context) :
  mApp{&context} { }


auto ProjectWindow::Draw() -> void {
  if (ImGui::Begin("Project", &mIsOpen, ImGuiWindowFlags_NoCollapse)) {
    if ((!ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) || (!mSelectedPathResDirRel.empty() && !exists(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel))) {
      mSelectedPathResDirRel.clear();
    }

    DrawFilesystemTree(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath(), {});

    std::filesystem::path const selectedPathAbs{
      mSelectedPathResDirRel.empty()
        ? mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath()
        : mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel
    };

    std::filesystem::path const workingDirAbs{
      is_directory(selectedPathAbs)
        ? selectedPathAbs
        : selectedPathAbs.parent_path()
    };

    auto constexpr contextMenuId{"Context Menu"};
    auto constexpr importModalId{"Importer Settings"};

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup(contextMenuId);
    }

    bool openImportModal{false};

    if (ImGui::BeginPopup(contextMenuId)) {
      if (ImGui::BeginMenu("New")) {
        if (ImGui::MenuItem("Folder")) {
          auto const newFolderPathAbs{GenerateUniquePath(workingDirAbs / "New Folder")};
          create_directory(newFolderPathAbs);

          mSelectedPathResDirRel = newFolderPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
          mApp->SetSelectedObject(nullptr);
          mRenameInfo = RenameInfo{.newName = newFolderPathAbs.stem().string(), .nodePathAbs = newFolderPathAbs};
        }

        if (ImGui::MenuItem("Material")) {
          mApp->GetResourceDatabase().CreateResource(*new Material{}, workingDirAbs / "New Material.mtl");
        }

        if (ImGui::MenuItem("Scene")) {
          mApp->GetResourceDatabase().CreateResource(*new Scene{}, workingDirAbs / "New Scene.scene");
        }

        ImGui::EndMenu();
      }


      if (ImGui::MenuItem("Import")) {
        if (std::filesystem::path srcPathAbs; OpenFileDialog("", "", srcPathAbs) && srcPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath()).empty()) {
          auto const dstPathAbs{workingDirAbs / srcPathAbs.filename()};
          copy_file(srcPathAbs, dstPathAbs);
          auto const dstPathResDirRel{dstPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())};

          if (auto const importerNode{mApp->GetResourceDatabase().FindImporterForResourceFile(dstPathResDirRel)}; !importerNode.IsNull()) {
            mImportModal.node = importerNode;
            mImportModal.dstPathResDirRel = dstPathResDirRel;

            openImportModal = true;
          }
        }
      }

      ImGui::EndPopup();
    }

    if (openImportModal) {
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
