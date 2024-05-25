#include "ProjectWindow.hpp"

#include "GUI.hpp"
#include "Material.hpp"
#include "Platform.hpp"
#include "ReflectionDisplayProperties.hpp"
#include "Util.hpp"
#include "engine_context.hpp"

#include <imgui_stdlib.h>
#include <nfd.h>

#include <cstring>
#include <optional>


namespace sorcery::mage {
auto ProjectWindow::DrawFilesystemTree(std::filesystem::path const& nodePathAbs,
                                       std::filesystem::path const& nodePathResDirRel,
                                       bool const isDirectory) noexcept -> bool {
  auto ret{false};

  auto& resDb{mApp->GetResourceDatabase()};
  auto const& resDirAbs{resDb.GetResourceDirectoryAbsolutePath()};

  auto thisPathResDirRel{nodePathResDirRel};
  auto thisPathAbs{nodePathAbs};
  auto selectedPathAbs{mSelectedPathResDirRel.empty() ? resDirAbs : resDirAbs / mSelectedPathResDirRel};

  auto const isSelected{thisPathAbs == selectedPathAbs};
  auto const isRenaming{mRenameInfo && mRenameInfo->nodePathAbs == thisPathAbs};

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
  auto treeNodeLabel{thisPathAbs.stem().string()};
  auto const nodeIsOpen{
    ImGui::TreeNodeEx((isRenaming ? treeNodeLabel.insert(0, "##") : treeNodeLabel).c_str(), treeNodeFlags)
  };

  if (isRenaming) {
    ImGui::SetKeyboardFocusHere();
    ImGui::SetCursorPos(treeNodePos);

    if (ImGui::InputText("##Rename", &mRenameInfo->newName,
      ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
      auto const newPathAbs{
        mRenameInfo->nodePathAbs.parent_path() / mRenameInfo->newName += mRenameInfo->nodePathAbs.extension()
      };
      auto const newPathResDirRel{newPathAbs.lexically_relative(resDirAbs)};

      if (isDirectory
            ? resDb.MoveDirectory(mRenameInfo->nodePathAbs.lexically_relative(resDirAbs), newPathResDirRel)
            : resDb.MoveResource(resDb.PathToGuid(thisPathResDirRel), newPathResDirRel)) {
        thisPathAbs = newPathAbs;
        thisPathResDirRel = thisPathAbs.lexically_relative(resDirAbs);
        mSelectedPathResDirRel = thisPathResDirRel;
        selectedPathAbs = resDirAbs / mSelectedPathResDirRel;
        ret = true;
      }

      mRenameInfo.reset();
    }

    if ((!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || ImGui::IsKeyPressed(
          ImGuiKey_Escape)) {
      mRenameInfo.reset();
    }
  } else if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (
               isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
    mSelectedPathResDirRel = thisPathResDirRel;
    selectedPathAbs = resDirAbs / mSelectedPathResDirRel;
    StartRenamingSelected();
  }

  if (!thisPathResDirRel.empty() && ImGui::BeginDragDropSource()) {
    if (isDirectory) {
      auto const thisPathResDirRelStr{thisPathResDirRel.string()};
      ImGui::SetDragDropPayload(DIR_NODE_DRAG_DROP_TYPE_STR.data(), thisPathResDirRelStr.c_str(),
        thisPathResDirRelStr.size() + 1);
    } else {
      auto const res{g_engine_context.resource_manager->GetOrLoad(resDb.PathToGuid(thisPathResDirRel))};
      ImGui::SetDragDropPayload(ObjectDragDropData::TYPE_STR.data(), &res, sizeof(decltype(res)));
    }
    ImGui::EndDragDropSource();
  }

  if (isDirectory && ImGui::BeginDragDropTarget()) {
    if (auto const payload{ImGui::AcceptDragDropPayload(DIR_NODE_DRAG_DROP_TYPE_STR.data())}) {
      std::string payloadPathResDirRelStr(static_cast<std::size_t>(payload->DataSize), '\0');
      std::memcpy(payloadPathResDirRelStr.data(), payload->Data, payload->DataSize);
      std::filesystem::path const payloadPathResDirRel{payloadPathResDirRelStr};

      if (resDb.MoveDirectory(payloadPathResDirRel, thisPathResDirRel / payloadPathResDirRel.filename())) {
        ret = true;
      }
    }

    if (auto const payload{ImGui::AcceptDragDropPayload(ObjectDragDropData::TYPE_STR.data())}) {
      if (auto const objectDragDropData{static_cast<ObjectDragDropData*>(payload->Data)};
        objectDragDropData && objectDragDropData->ptr && rttr::type::get(*objectDragDropData->ptr).is_derived_from(
          rttr::type::get<Resource>())) {
        if (auto const res{static_cast<Resource*>(objectDragDropData->ptr)}; resDb.MoveResource(res->GetGuid(),
          thisPathResDirRel / resDb.GuidToPath(res->GetGuid()).filename())) {
          ret = true;
        }
      }
    }

    ImGui::EndDragDropTarget();
  }

  if ((ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) || ImGui::IsItemClicked(
        ImGuiMouseButton_Right)) {
    mSelectedPathResDirRel = thisPathResDirRel;
    selectedPathAbs = resDirAbs / mSelectedPathResDirRel;
    mApp->SetSelectedObject(g_engine_context.resource_manager->GetOrLoad(resDb.PathToGuid(thisPathResDirRel)));
  }

  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    mOpenContextMenu = true;
  }

  if (nodeIsOpen) {
    if (isDirectory) {
      for (auto const& entry : std::filesystem::directory_iterator{thisPathAbs}) {
        if (entry.path().extension() != ResourceDB::RESOURCE_META_FILE_EXT) {
          if (DrawFilesystemTree(entry.path(), thisPathResDirRel / entry.path().filename(), entry.is_directory())) {
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
  if (ImGui::BeginPopup(CONTEXT_MENU_ID.data())) {
    auto const selectedPathAbs{
      weakly_canonical(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel)
    };
    auto const workingDirAbs{is_directory(selectedPathAbs) ? selectedPathAbs : selectedPathAbs.parent_path()};
    auto const isResDirSelected{
      exists(selectedPathAbs) && exists(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath()) && equivalent(
        selectedPathAbs, mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())
    };

    if (ImGui::BeginMenu("New")) {
      if (ImGui::MenuItem("Folder")) {
        auto const newFolderPathAbs{GenerateUniquePath(workingDirAbs / "New Folder")};
        create_directory(newFolderPathAbs);

        mSelectedPathResDirRel = newFolderPathAbs.lexically_relative(
          mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
        mApp->SetSelectedObject(nullptr);
      }

      if (ImGui::MenuItem("Material")) {
        auto const mtl{Create<Material>().release()};
        auto const mtlPathAbs{GenerateUniquePath(workingDirAbs / "New Material.mtl")};
        mApp->GetResourceDatabase().CreateResource(*mtl, mtlPathAbs);
        mSelectedPathResDirRel = mtlPathAbs.lexically_relative(
          mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath());
        mApp->SetSelectedObject(mtl);
      }

      if (ImGui::MenuItem("Scene")) {
        auto const scene{Create<Scene>().release()};
        auto const scenePathAbs{GenerateUniquePath(workingDirAbs / "New Scene.scene")};
        mApp->GetResourceDatabase().CreateResource(*scene, scenePathAbs);
        mApp->SetSelectedObject(scene);
      }

      ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Import")) {
      if (nfdpathset_t pathSet; NFD_OpenDialogMultiple("", "", &pathSet) == NFD_OKAY) {
        mFilesToImport.clear();
        mOpenImportModal = false;

        for (std::size_t i{0}; i < NFD_PathSet_GetCount(&pathSet); i++) {
          std::filesystem::path const srcPathAbs{NFD_PathSet_GetPath(&pathSet, i)};
          if (auto importer{ResourceDB::GetNewImporterForResourceFile(srcPathAbs)}) {
            mFilesToImport.emplace_back(std::move(importer), srcPathAbs,
              GenerateUniquePath(workingDirAbs / srcPathAbs.filename()));
            mOpenImportModal = true;
          } else {
            ImGui::EndPopup();
            throw std::runtime_error{
              std::format("Couldn't find importer for file type {}.", srcPathAbs.extension().string())
            };
          }
        }

        NFD_PathSet_Free(&pathSet);
      }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Import Settings", nullptr, nullptr, !is_directory(selectedPathAbs))) {
      if (std::unique_ptr<ResourceImporter> importer; ResourceDB::LoadMeta(selectedPathAbs, nullptr,
        std::addressof(importer))) {
        mFilesToImport.emplace_back(std::move(importer), selectedPathAbs, selectedPathAbs);
        mOpenImportModal = true;
      }
    }

    if (ImGui::MenuItem("Rename", nullptr, nullptr, !isResDirSelected)) {
      StartRenamingSelected();
    }

    if (ImGui::MenuItem("Delete", nullptr, nullptr, !isResDirSelected)) {
      if (is_directory(selectedPathAbs)) {
        if (mApp->GetResourceDatabase().DeleteDirectory(selectedPathAbs)) {
          mApp->SetSelectedObject(nullptr);
          mSelectedPathResDirRel.clear();
        }
      } else {
        mApp->GetResourceDatabase().DeleteResource(mApp->GetResourceDatabase().PathToGuid(mSelectedPathResDirRel));
        mApp->SetSelectedObject(nullptr);
        mSelectedPathResDirRel.clear();
      }
    }

    ImGui::EndPopup();
  }
}


auto ProjectWindow::StartRenamingSelected() noexcept -> void {
  mRenameInfo = RenameInfo{
    .newName = mSelectedPathResDirRel.stem().string(),
    .nodePathAbs = mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel
  };
}


ProjectWindow::ProjectWindow(Application& context) :
  mApp{&context} {}


auto ProjectWindow::Draw() -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{150, 150}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoCollapse)) {
    if ((!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows) &&
         ImGui::IsAnyItemHovered() && (ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
                                       ImGui::IsMouseReleased(ImGuiMouseButton_Right))) || (
          !mSelectedPathResDirRel.empty() && !exists(
            mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath() / mSelectedPathResDirRel))) {
      mSelectedPathResDirRel.clear();
    }

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        mSelectedPathResDirRel.clear();
      }

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        mOpenContextMenu = true;
      }
    }

    std::ignore = DrawFilesystemTree(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath(), "", true);

    if (mOpenContextMenu) {
      mOpenContextMenu = false;
      ImGui::OpenPopup(CONTEXT_MENU_ID.data());
    }

    try {
      DrawContextMenu();
    } catch ([[maybe_unused]] std::runtime_error const& ex) {
      ImGui::End();
      throw;
    }

    auto constexpr importModalId{"Import Settings"};

    if (mOpenImportModal) {
      mOpenImportModal = false;
      ImGui::OpenPopup(importModalId);
    }

    if (ImGui::BeginPopupModal(importModalId)) {
      for (auto& [importer, srcPathAbs, dstPathAbs] : mFilesToImport) {
        ImGui::SeparatorText(srcPathAbs.stem().string().c_str());
        ImGui::PushID(srcPathAbs.string().c_str());
        ReflectionDisplayProperties(*importer);
        ImGui::PopID();
      }

      if (ImGui::Button("Cancel")) {
        mFilesToImport.clear();
        ImGui::CloseCurrentPopup();
      }

      ImGui::SameLine();

      if (ImGui::Button("Import")) {
        for (auto const& [importer, srcPathAbs, dstPathAbs] : mFilesToImport) {
          if (exists(srcPathAbs) && !exists(dstPathAbs)) {
            copy_file(srcPathAbs, dstPathAbs);
          }

          if (!mApp->GetResourceDatabase().ImportResource(
            dstPathAbs.lexically_relative(mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath()),
            importer.get())) {
            remove(dstPathAbs);
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::End();
            throw std::runtime_error{std::format("Failed to import {}.", dstPathAbs.string())};
          }
        }
        mFilesToImport.clear();
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }
  }
  ImGui::End();
}
}
