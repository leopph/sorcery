// ReSharper disable All
#define _CRT_SECURE_NO_WARNINGS
// ReSharper restore All
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ProjectWindow.hpp"
#include "Util.hpp"
#include "Platform.hpp"

#include <nfd.h>
#include <stb_image.h>
#include <stb_image_write.h>
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


auto ProjectWindow::ImportConcreteAsset(Context& context, AssetLoader& assetLoader, std::filesystem::path const& srcPathAbs, std::filesystem::path const& selectedDirAbs) -> void {
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


auto ProjectWindow::ImportAsset(Context& context, Object::Type const targetAssetType, std::filesystem::path const& selectedDirAbs) -> void {
  auto& loader{ context.GetFactoryManager().GetFor(targetAssetType).GetLoader() };

  if (std::filesystem::path path; OpenFileDialog(Join(loader.GetSupportedExtensions(), ",").c_str(), nullptr, path)) {
    ImportConcreteAsset(context, loader, absolute(path), selectedDirAbs);
  }
}


auto ProjectWindow::SaveNewNativeAsset(Context& context, std::unique_ptr<NativeResource> asset, std::string_view const targetAssetFileName, std::filesystem::path const& selectedDirAbs) -> void {
  auto const dst{ GenerateUniquePath(selectedDirAbs / targetAssetFileName) };

  asset->SetName(dst.stem().string());
  asset->SetGuid(Guid::Generate());

  auto const& assetRef{ *asset };

  context.GetResources().RegisterAsset(std::move(asset), dst);
  context.SaveRegisteredNativeAsset(assetRef);
  context.CreateMetaFileForRegisteredAsset(assetRef);
}


ProjectWindow::ProjectWindow(Context& context) :
  mContext{ &context } { }


auto ProjectWindow::Draw() -> void {
  if (ImGui::Begin("Project", &mIsOpen, ImGuiWindowFlags_NoCollapse)) {
    if ((!ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) || (mSelectedNodePathProjDirRel && !exists(mContext->GetProjectDirectoryAbsolute() / *mSelectedNodePathProjDirRel))) {
      mSelectedNodePathProjDirRel = std::nullopt;
    }

    DrawFilesystemTree(mContext->GetProjectDirectoryAbsolute(), Context::GetAssetDirectoryProjectRootRelative(), true, nullptr, mSelectedNodePathProjDirRel);

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
      if (ImGui::BeginMenu("New##Menu")) {
        if (ImGui::MenuItem("Folder")) {
          auto const newFolderPath{ GenerateUniquePath(targetWorkingDirPathAbs / "New Folder") };
          create_directory(newFolderPath);
          mSelectedNodePathProjDirRel = relative(newFolderPath, mContext->GetAssetDirectoryAbsolute());
          mContext->SetSelectedObject(nullptr);
          mRenameInfo = RenameInfo{ .newName = newFolderPath.stem().string(), .nodePathAbs = newFolderPath };
        }

        if (ImGui::MenuItem("Material")) {
          SaveNewNativeAsset(*mContext, std::unique_ptr<NativeResource>{ new Material{} }, "New Material.mtl", targetWorkingDirPathAbs);
        }

        if (ImGui::MenuItem("Scene##CreateSceneAsset")) {
          SaveNewNativeAsset(*mContext, std::unique_ptr<NativeResource>{ new Scene{} }, "New Scene.scene", targetWorkingDirPathAbs);
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Import##ImportAssetMenu")) {
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

    std::string static cubeMapcombinedFileName;
    std::array<std::filesystem::path, 6> static facePaths;

    if (openCubemapImportModal) {
      for (auto& path : facePaths) {
        path.clear();
      }
      cubeMapcombinedFileName.clear();
      ImGui::OpenPopup(CUBEMAP_IMPORT_MODAL_ID);
    }

    if (ImGui::BeginPopupModal(CUBEMAP_IMPORT_MODAL_ID, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      constexpr auto importTypeCount{ 2 };
      constexpr std::array<char const*, importTypeCount> importTypeNames{ "From Single File", "Merge Separate Faces" };
      static auto importTypeIdx{ 0 };

      ImGui::Combo("##CubeMapImportTypeCombo", &importTypeIdx, importTypeNames.data(), importTypeCount);

      if (importTypeIdx == 1 && ImGui::BeginTable("CubeMapImportModalTable", 2)) {
        // Order counts here
        constexpr std::array faceNames{ "Right Face", "Left Face", "Top Face", "Bottom Face", "Front Face", "Back Face", };

        for (int i = 0; i < 6; i++) {
          ImGui::PushID(i);
          ImGui::TableNextColumn();
          ImGui::Text("%s", faceNames[i]);
          ImGui::TableNextColumn();
          if (ImGui::Button("Select")) {
            std::ignore = OpenFileDialog(Join(mContext->GetFactoryManager().GetFor<Cubemap>().GetLoader().GetSupportedExtensions(), ",").c_str(), nullptr, facePaths[i]);
          }
          ImGui::SameLine();
          ImGui::Text("%s", facePaths[i].empty()
                              ? "None"
                              : facePaths[i].filename().string().c_str());
          ImGui::PopID();
        }

        ImGui::TableNextColumn();
        ImGui::Text("%s", "Name of combined file");
        ImGui::TableNextColumn();
        ImGui::InputText("##CombinedFileInputText", &cubeMapcombinedFileName);

        ImGui::EndTable();
      }

      if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
      }

      ImGui::SameLine();

      if (ImGui::Button("Import")) {
        if (importTypeIdx == 0) {
          ImportAsset(*mContext, Object::Type::Cubemap, targetWorkingDirPathAbs);
          ImGui::CloseCurrentPopup();
        } else if (importTypeIdx == 1) {
          bool rdyToImport{ true };

          if (cubeMapcombinedFileName.empty()) {
            rdyToImport = false;
            DisplayError("Please enter the name of the combined image file.");
          }

          auto const dstPath{ targetWorkingDirPathAbs / std::filesystem::path{ cubeMapcombinedFileName } += ".png" };

          if (exists(dstPath)) {
            rdyToImport = false;
            DisplayError("The filename is already in use.");
          }

          for (auto& path : facePaths) {
            if (path.empty()) {
              rdyToImport = false;
              DisplayError("Not all faces of the cubemap are selected. Please select all of them.");
              break;
            }
          }

          if (rdyToImport) {
            std::array<Image, 6> loadedImages;

            bool importSuccess{ true };

            for (int i = 0; i < 6; i++) {
              int width;
              int height;
              int channelCount;

              if (auto const data{ stbi_load(facePaths[i].string().c_str(), &width, &height, &channelCount, 4) }) {
                if (width != height) {
                  importSuccess = false;
                  DisplayError(std::format(L"Image at {} cannot be used as a cubemap face because it is not square.", facePaths[i].wstring()));
                  stbi_image_free(data);
                  break;
                }

                if (!IsPowerOfTwo(width)) {
                  importSuccess = false;
                  DisplayError(std::format(L"Image at {} cannot be used as a cubemap face because its dimensions are not powers of two.", facePaths[i].wstring()));
                  stbi_image_free(data);
                  break;
                }

                if (i > 0 && static_cast<int>(loadedImages[i - 1].get_width()) != width) {
                  importSuccess = false;
                  DisplayError(std::format(L"The image dimensions at {} don't match the dimensions of the previous images.", facePaths[i].wstring()));
                  stbi_image_free(data);
                  break;
                }

                loadedImages[i] = Image{ static_cast<u32>(width), static_cast<u32>(height), 4, std::unique_ptr<u8[]>{ data } };
              } else {
                importSuccess = false;
                DisplayError(std::format(L"Failed to load image file at {}.", facePaths[i].wstring()));
                break;
              }
            }

            if (importSuccess) {
              auto const faceSize{ static_cast<int>(loadedImages[0].get_width()) };
              auto combinedBytes{ std::make_unique_for_overwrite<u8[]>(6 * faceSize * faceSize * 4) };
              for (int i = 0; i < 6; i++) {
                std::ranges::copy_n(loadedImages[i].get_data().data(), faceSize * faceSize * 4, combinedBytes.get() + i * faceSize * faceSize * 4);
              }
              if (!stbi_write_png(dstPath.string().c_str(), faceSize, 6 * faceSize, 4, combinedBytes.get(), faceSize * 4)) {
                DisplayError("Failed to write combined image.");
              } else {
                ImportConcreteAsset(*mContext, mContext->GetFactoryManager().GetFor<Cubemap>().GetLoader(), dstPath, targetWorkingDirPathAbs);
                ImGui::CloseCurrentPopup();
              }
            }
          }
        }
      }

      ImGui::EndPopup();
    }
  }
  ImGui::End();
}
}
