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
auto ProjectWindow::DrawFilesystemTree(std::filesystem::path const& rootDirAbs, std::filesystem::path const& thisNodePathRootRel, bool const isThisNodeDirectory, Object* assetAtThisNode, std::optional<std::filesystem::path>& selectedNodePathRootRel) -> void {
  auto nodePathAbs{ rootDirAbs / thisNodePathRootRel };
  auto const isSelected{ selectedNodePathRootRel && equivalent(nodePathAbs, rootDirAbs / *selectedNodePathRootRel) };
  auto const isRenaming{ mRenameInfo && equivalent(mRenameInfo->nodePathAbs, nodePathAbs) };

  ImGuiTreeNodeFlags treeNodeFlags{ ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick };

  if (!isThisNodeDirectory) {
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
                                              : "", nodePathAbs.stem().string()).c_str(), treeNodeFlags)) {
    if (ImGui::IsItemClicked()) {
      selectedNodePathRootRel = thisNodePathRootRel;
      mContext->SetSelectedObject(assetAtThisNode);
    }

    if (isRenaming) {
      ImGui::SetKeyboardFocusHere();
      ImGui::SetCursorPos(treeNodePos);

      if (ImGui::InputText("##Rename", &mRenameInfo->newName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
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
          selectedNodePathRootRel = relative(newNodePathAbs, rootDirAbs);
        }

        nodePathAbs = newNodePathAbs;
      }

      if ((!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        mRenameInfo.reset();
      }
    } else if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
      mRenameInfo = RenameInfo{ .newName = nodePathAbs.stem().string(), .nodePathAbs = nodePathAbs };
    }

    if (isThisNodeDirectory) {
      for (auto const& entry : std::filesystem::directory_iterator{ nodePathAbs }) {
        auto const entryIsDirectory{ entry.is_directory() };

        if (auto const assetAtEntry{
          entryIsDirectory
            ? nullptr
            : mContext->GetResources().TryGetAssetAt(absolute(entry.path()))
        }; entryIsDirectory || assetAtEntry) {
          auto const rootRelativeChildDir{ relative(entry.path(), rootDirAbs) };
          DrawFilesystemTree(rootDirAbs, rootRelativeChildDir, entryIsDirectory, assetAtEntry, selectedNodePathRootRel);
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


auto ProjectWindow::ImportConcreteAsset(Application& context, AssetLoader& assetLoader, std::filesystem::path const& srcPathAbs, std::filesystem::path const& selectedDirAbs) -> void {
  context.ExecuteInBusyEditor([&context, &assetLoader, srcPathAbs, selectedDirAbs] {
    auto const dstPath{
      equivalent(srcPathAbs.parent_path(), selectedDirAbs)
        ? srcPathAbs
        : GenerateUniquePath(selectedDirAbs / srcPathAbs.filename())
    };

    if (!exists(dstPath) || !equivalent(dstPath, srcPathAbs)) {
      copy_file(srcPathAbs, dstPath);
    }

    auto const guid{ Guid::Generate() };

    if (auto asset{ assetLoader.Load(dstPath, context.GetCacheDirectoryAbsolute()) }) {
      asset->SetName(dstPath.stem().string());
      asset->SetGuid(guid);

      context.GetResources().RegisterAsset(std::shared_ptr<Resource>{ asset.release() }, dstPath);
      context.CreateMetaFileForRegisteredAsset(*asset);
    } else {
      throw std::runtime_error{ std::format("Failed to import asset at {}.", srcPathAbs.string()) };
    }
  });
}


auto ProjectWindow::ImportAsset(Application& context, Object::Type const targetAssetType, std::filesystem::path const& selectedDirAbs) -> void {
  auto& loader{ context.GetFactoryManager().GetFor(targetAssetType).GetLoader() };

  if (std::filesystem::path path; OpenFileDialog(Join(loader.GetSupportedExtensions(), ",").c_str(), nullptr, path)) {
    ImportConcreteAsset(context, loader, absolute(path), selectedDirAbs);
  }
}


auto ProjectWindow::SaveNewNativeAsset(Application& context, std::unique_ptr<NativeResource> asset, std::string_view const targetAssetFileName, std::filesystem::path const& selectedDirAbs) -> void {
  auto const dst{ GenerateUniquePath(selectedDirAbs / targetAssetFileName) };

  asset->SetName(dst.stem().string());
  asset->SetGuid(Guid::Generate());

  auto const& assetRef{ *asset };

  context.GetResources().RegisterAsset(std::move(asset), dst);
  context.SaveRegisteredNativeAsset(assetRef);
  context.CreateMetaFileForRegisteredAsset(assetRef);
}


ProjectWindow::ProjectWindow(Application& context) :
  mContext{ &context } { }


auto ProjectWindow::Draw() -> void {
  if (ImGui::Begin("Project", &mIsOpen, ImGuiWindowFlags_NoCollapse)) {
    if ((!ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) || (mSelectedNodePathProjDirRel && !exists(mContext->GetProjectDirectoryAbsolute() / *mSelectedNodePathProjDirRel))) {
      mSelectedNodePathProjDirRel = std::nullopt;
    }

    DrawFilesystemTree(mContext->GetProjectDirectoryAbsolute(), Application::GetAssetDirectoryProjectRootRelative(), true, nullptr, mSelectedNodePathProjDirRel);

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup(CONTEXT_MENU_ID);
    }

    std::filesystem::path const targetWorkingDirPathAbs{
      mSelectedNodePathProjDirRel
        ? is_directory(*mSelectedNodePathProjDirRel)
            ? mContext->GetAssetDirectoryAbsolute() / *mSelectedNodePathProjDirRel
            : (mContext->GetAssetDirectoryAbsolute() / *mSelectedNodePathProjDirRel).parent_path()
        : mContext->GetAssetDirectoryAbsolute()
    };

    auto openCubemapImportModal{ false };

    if (ImGui::BeginPopup(CONTEXT_MENU_ID)) {
      if (ImGui::BeginMenu("New")) {
        if (ImGui::MenuItem("Folder")) {
          auto const newFolderPath{ GenerateUniquePath(targetWorkingDirPathAbs / "New Folder") };
          create_directory(newFolderPath);
          mSelectedNodePathProjDirRel = relative(newFolderPath, mContext->GetAssetDirectoryAbsolute());
          mContext->SetSelectedObject(nullptr);
          mRenameInfo = RenameInfo{ .newName = newFolderPath.stem().string(), .nodePathAbs = newFolderPath };
        }

        if (ImGui::MenuItem("Material")) {
          mContext->GetResourceDatabase().CreateResource(new Material{}, "New Material.mtl");
        }

        if (ImGui::MenuItem("Scene##CreateSceneAsset")) {
          mContext->GetResourceDatabase().CreateResource(new Scene{}, "New Scene.scene");
        }

        ImGui::EndMenu();
      }

      if (ImGui::MenuItem("Import")) {
        if (ImGui::MenuItem("Mesh##ImportMeshAssetMenuItem")) {
          ImportAsset(*mContext, Object::Type::Mesh, targetWorkingDirPathAbs);
          ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem("2D Texture##Import2DTextureAssetMenuItem")) {
          ImportAsset(*mContext, Object::Type::Texture2D, targetWorkingDirPathAbs);
          ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem("Cubemap Texture##ImportCubemapTextureAssetMenuItem")) {
          openCubemapImportModal = true;
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndMenu();
      }

      ImGui::EndPopup();
    }
  }
  ImGui::End();
}
}
