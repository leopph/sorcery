// ReSharper disable All
#define _CRT_SECURE_NO_WARNINGS
// ReSharper restore All
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ProjectWindow.hpp"

#include "Material.hpp"
#include "Util.hpp"
#include "Platform.hpp"

#include <nfd.h>
#include <imgui_stdlib.h>

#include <optional>


namespace sorcery::mage {
auto ProjectWindow::DrawFilesystemTree(std::filesystem::path const& resDirAbs, std::filesystem::path const& thisPathResDirRel) -> void {
  auto const pathAbs{ resDirAbs / thisPathResDirRel };
  auto const isSelected{ equivalent(pathAbs, resDirAbs / mSelectedPathResDirRel) };
  auto const isRenaming{ mRenameInfo && equivalent(mRenameInfo->nodePathAbs, pathAbs) };
  auto const isDirectory{ is_directory(pathAbs) };

  ImGuiTreeNodeFlags treeNodeFlags{ ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick };

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

  auto const treeNodePos{ ImGui::GetCursorPos() };

  if (ImGui::TreeNodeEx(std::format("{}{}", isRenaming
                                              ? "##"
                                              : "", pathAbs.stem().string()).c_str(), treeNodeFlags)) {
    if (ImGui::IsItemClicked()) {
      mSelectedPathResDirRel = thisPathResDirRel;
      mContext->SetSelectedObject(gResourceManager.LoadResource(mContext->GetResourceDatabase().PathToGuid(pathAbs)));
    }

    if (isRenaming) {
      ImGui::SetKeyboardFocusHere();
      ImGui::SetCursorPos(treeNodePos);

      /* TODO if (ImGui::InputText("##Rename", &mRenameInfo->newName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
        auto const newNodePathAbs{ mRenameInfo->nodePathAbs.parent_path() / mRenameInfo->newName += mRenameInfo->nodePathAbs.extension() };

        if (!isThisNodeDirectory) {
          auto renamedAsset{ mContext->GetResources().UnregisterAsset(mRenameInfo->nodePathAbs) };

          auto const oldAssetMetaPath{ std::filesystem::path{ mRenameInfo->nodePathAbs } += mContext->GetAssetFileExtension() };
          auto const newAssetMetaPath{ std::filesystem::path{ newNodePathAbs } += mContext->GetAssetFileExtension() };
          std::filesystem::rename(oldAssetMetaPath, newAssetMetaPath);

          renamedAsset->SetName(newNodePathAbs.stem().string());
          mContext->GetResources().RegisterAsset(std::move(renamedAsset), newNodePathAbs);
        }

        std::filesystem::rename(mRenameInfo->nodePathAbs, newNodePathAbs);
        mRenameInfo.reset();

        if (isSelected) {
          selectedPathResDirRel = relative(newNodePathAbs, resDirAbs);
        }

        nodePathAbs = newNodePathAbs;
      }*/

      if ((!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        mRenameInfo.reset();
      }
    } else if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
      mRenameInfo = RenameInfo{ .newName = pathAbs.stem().string(), .nodePathAbs = pathAbs };
    }

    if (isDirectory) {
      for (auto const& entry : std::filesystem::directory_iterator{ pathAbs }) {
        if (entry.is_directory()) {
          DrawFilesystemTree(resDirAbs, relative(entry.path(), resDirAbs));
        }
      }
    }

    ImGui::TreePop();
  }
}


auto ProjectWindow::OpenFileDialog(std::string_view const filters, std::string_view const defaultPath, std::filesystem::path& out) -> bool {
  if (nfdchar_t* selectedPath{ nullptr }; NFD_OpenDialog(filters.data(), defaultPath.data(), &selectedPath) == NFD_OKAY) {
    out = selectedPath;
    std::free(selectedPath);
    return true;
  }

  return false;
}


ProjectWindow::ProjectWindow(Application& context) :
  mContext{ &context } { }


auto ProjectWindow::Draw() -> void {
  if (ImGui::Begin("Project", &mIsOpen, ImGuiWindowFlags_NoCollapse)) {
    if ((!ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) || (!mSelectedPathResDirRel.empty() && !exists(mContext->GetProjectDirectoryAbsolute() / mSelectedPathResDirRel))) {
      mSelectedPathResDirRel.clear();
    }

    DrawFilesystemTree(mContext->GetResourceDatabase().GetResourceDirectoryAbsolutePath(), "");

    std::filesystem::path const selectedPathAbs{
      mSelectedPathResDirRel.empty()
        ? mContext->GetResourceDatabase().GetResourceDirectoryAbsolutePath()
        : mContext->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel
    };

    std::filesystem::path const workingDirAbs{
      is_directory(selectedPathAbs)
        ? selectedPathAbs
        : selectedPathAbs.parent_path()
    };

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup(CONTEXT_MENU_ID);
    }

    if (ImGui::BeginPopup(CONTEXT_MENU_ID)) {
      if (ImGui::BeginMenu("New")) {
        if (ImGui::MenuItem("Folder")) {
          auto const newFolderPathAbs{ GenerateUniquePath(workingDirAbs / "New Folder") };
          create_directory(newFolderPathAbs);

          mSelectedPathResDirRel = newFolderPathAbs.lexically_relative(mContext->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
          mContext->SetSelectedObject(nullptr);
          mRenameInfo = RenameInfo{ .newName = newFolderPathAbs.stem().string(), .nodePathAbs = newFolderPathAbs };
        }

        if (ImGui::MenuItem("Material")) {
          mContext->GetResourceDatabase().CreateResource(*new Material{}, workingDirAbs / "New Material.mtl");
        }

        if (ImGui::MenuItem("Scene##CreateSceneAsset")) {
          mContext->GetResourceDatabase().CreateResource(*new Scene{}, workingDirAbs / "New Scene.scene");
        }

        ImGui::EndMenu();
      }

      if (ImGui::MenuItem("Import")) {
        if (std::filesystem::path srcPathAbs; OpenFileDialog("", "", srcPathAbs)) {
          if (srcPathAbs.lexically_relative(mContext->GetResourceDatabase().GetResourceDirectoryAbsolutePath()).empty()) {
            auto const dstPathAbs{ workingDirAbs / srcPathAbs.filename() };
            copy_file(srcPathAbs, dstPathAbs);
            mContext->GetResourceDatabase().ImportResource(dstPathAbs.lexically_relative(mContext->GetResourceDatabase().GetResourceDirectoryAbsolutePath()));
          }
        }

        ImGui::EndMenu();
      }

      ImGui::EndPopup();
    }
  }
  ImGui::End();
}
}
