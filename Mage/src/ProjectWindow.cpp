#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ProjectWindow.hpp"
#include "Util.hpp"

#include <nfd.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <imgui_stdlib.h>

#include <functional>
#include <optional>

#include "Platform.hpp"


namespace sorcery::mage {
namespace {
auto DrawSideFolderViewNodeRecursive(std::filesystem::path const& absoluteRootDir, std::filesystem::path const& rootRelativeNodeDir, std::filesystem::path& currentRootRelativeDir) -> void {
  auto const absoluteNodeDir{ canonical(absoluteRootDir / rootRelativeNodeDir) };

  std::filesystem::directory_iterator const it{ absoluteNodeDir };
  ImGuiTreeNodeFlags treeNodeFlags{ ImGuiTreeNodeFlags_OpenOnArrow };

  if (begin(it) == end(it)) {
    treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
  } else {
    treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
  }

  if (equivalent(absoluteNodeDir, canonical(absoluteRootDir / currentRootRelativeDir))) {
    treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
  }

  if (ImGui::TreeNodeEx(absoluteNodeDir.stem().string().c_str(), treeNodeFlags)) {
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      currentRootRelativeDir = rootRelativeNodeDir;
    }

    for (auto const& entry : it) {
      if (is_directory(entry)) {
        auto const rootRelativeChildDir{ relative(entry.path(), absoluteRootDir) };
        DrawSideFolderViewNodeRecursive(absoluteRootDir, rootRelativeChildDir, currentRootRelativeDir);
      }
    }
    ImGui::TreePop();
  }
}


auto DrawSideFolderView(std::filesystem::path const& absoluteAssetDir, std::filesystem::path& currentRootRelativeDir) -> void {
  DrawSideFolderViewNodeRecursive(absoluteAssetDir, "", currentRootRelativeDir);
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
      equivalent(srcPathAbs.parent_path(), selectedDirAbs) ? srcPathAbs : GenerateUniquePath(selectedDirAbs / srcPathAbs.filename())
    };

    if (!exists(dstPath) || !equivalent(dstPath, srcPathAbs)) {
      copy_file(srcPathAbs, dstPath);
    }

    auto const guid{ Guid::Generate() };

    if (auto asset{ assetLoader.Load(dstPath, context.GetCacheDirectoryAbsolute()) }) {
      asset->SetName(dstPath.stem().string());
      asset->SetGuid(guid);

      context.GetResources().RegisterAsset(std::shared_ptr<Object>{ asset.release() }, dstPath);
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


auto ProjectWindow::SaveNewNativeAsset(Context& context, std::unique_ptr<NativeAsset> asset, std::string_view const targetAssetFileName, std::filesystem::path const& selectedDirAbs) -> void {
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
    if (ImGui::BeginTable("ProjectWindowMainTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit)) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      if (!exists(mContext->GetAssetDirectoryAbsolute() / mSelectedDirRel)) {
        mSelectedDirRel = "";
      }

      DrawSideFolderView(mContext->GetAssetDirectoryAbsolute(), mSelectedDirRel);

      auto const selectedDirAbs{ mContext->GetAssetDirectoryAbsolute() / mSelectedDirRel };

      ImGui::TableSetColumnIndex(1);

      auto constexpr contextMenuId{ "ProjectWindowContextMenu" };

      if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(contextMenuId);
      }

      auto openCubemapImportModal{ false };

      struct RenameInfo {
        std::string newName;
        std::filesystem::path src;
      };

      static std::optional<RenameInfo> renaming;
      static std::optional<std::filesystem::path> selectedDir;

      if (ImGui::BeginPopup(contextMenuId)) {
        if (ImGui::BeginMenu("New##Menu")) {
          if (ImGui::MenuItem("Folder")) {
            auto const newFolderPath{ GenerateUniquePath(selectedDirAbs / "New Folder") };
            create_directory(newFolderPath);
            selectedDir = newFolderPath;
            mContext->SetSelectedObject(nullptr);
            renaming = RenameInfo{ .newName = newFolderPath.stem().string(), .src = newFolderPath };
          }

          if (ImGui::MenuItem("Material")) {
            SaveNewNativeAsset(*mContext, std::unique_ptr<NativeAsset>{ new Material{} }, "New Material.mtl", selectedDirAbs);
          }

          if (ImGui::MenuItem("Scene##CreateSceneAsset")) {
            SaveNewNativeAsset(*mContext, std::unique_ptr<NativeAsset>{ new Scene{} }, "New Scene.scene", selectedDirAbs);
          }

          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Import##ImportAssetMenu")) {
          if (ImGui::MenuItem("Mesh##ImportMeshAssetMenuItem")) {
            ImportAsset(*mContext, Object::Type::Mesh, selectedDirAbs);
            ImGui::CloseCurrentPopup();
          }

          if (ImGui::MenuItem("2D Texture##Import2DTextureAssetMenuItem")) {
            ImportAsset(*mContext, Object::Type::Texture2D, selectedDirAbs);
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

      auto constexpr cubemapImportModalId{ "Import Cubemap" };
      std::string static cubeMapcombinedFileName;
      auto constexpr faceCount{ 6 };
      std::array<std::filesystem::path, faceCount> static facePaths;

      if (openCubemapImportModal) {
        for (auto& path : facePaths) {
          path.clear();
        }
        cubeMapcombinedFileName.clear();
        ImGui::OpenPopup(cubemapImportModalId);
      }

      if (ImGui::BeginPopupModal(cubemapImportModalId, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        constexpr auto importTypeCount{ 2 };
        constexpr std::array<char const*, importTypeCount> importTypeNames{ "From Single File", "Merge Separate Faces" };
        static auto importTypeIdx{ 0 };

        ImGui::Combo("##CubeMapImportTypeCombo", &importTypeIdx, importTypeNames.data(), importTypeCount);

        auto const drawFileSelectionEntries{
          [this]<int ElemCount>(std::span<char const* const, ElemCount> labels, std::span<std::filesystem::path, ElemCount> paths) {
            for (int i = 0; i < ElemCount; i++) {
              ImGui::PushID(i);
              ImGui::TableNextColumn();
              ImGui::Text("%s", labels[i]);
              ImGui::TableNextColumn();
              if (ImGui::Button("Select")) {
                std::ignore = OpenFileDialog(Join(mContext->GetFactoryManager().GetFor<Cubemap>().GetLoader().GetSupportedExtensions(), ",").c_str(), nullptr, paths[i]);
              }
              ImGui::SameLine();
              ImGui::Text("%s", paths[i].empty() ? "None" : paths[i].filename().string().c_str());
              ImGui::PopID();
            }
          }
        };

        if (importTypeIdx == 1 && ImGui::BeginTable("CubeMapImportModalTable", 2)) {
          // Order counts here
          constexpr std::array<char const*, faceCount> faceNames{ "Right Face", "Left Face", "Top Face", "Bottom Face", "Front Face", "Back Face", };
          drawFileSelectionEntries(std::span{ faceNames }, std::span{ facePaths });

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
            ImportAsset(*mContext, Object::Type::Cubemap, selectedDirAbs);
            ImGui::CloseCurrentPopup();
          } else if (importTypeIdx == 1) {
            bool rdyToImport{ true };

            if (cubeMapcombinedFileName.empty()) {
              rdyToImport = false;
              DisplayError("Please enter the name of the combined image file.");
            }

            auto const dstPath{ selectedDirAbs / std::filesystem::path{ cubeMapcombinedFileName } += ".png" };

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
                  ImportConcreteAsset(*mContext, mContext->GetFactoryManager().GetFor<Cubemap>().GetLoader(), dstPath, selectedDirAbs);
                  ImGui::CloseCurrentPopup();
                }
              }
            }
          }
        }

        ImGui::EndPopup();
      }

      for (auto const& entry : std::filesystem::directory_iterator{ selectedDirAbs }) {
        auto const entryAbsPath{ absolute(entry.path()) };

        if (is_directory(entryAbsPath)) {
          auto isSelected{ selectedDir && equivalent(*selectedDir, entryAbsPath) };

          if (renaming && equivalent(renaming->src, entryAbsPath)) {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##Rename", &renaming->newName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
              auto const newDirName{ renaming->src.parent_path() / renaming->newName };
              std::filesystem::rename(renaming->src, newDirName);
              renaming.reset();

              if (isSelected) {
                selectedDir = newDirName;
              }
            }
            if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
              renaming.reset();
            }
          } else {
            if (ImGui::Selectable(entryAbsPath.stem().string().c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
              if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                mContext->SetSelectedObject(nullptr);
                selectedDir.reset();
                mSelectedDirRel = relative(entryAbsPath, mContext->GetAssetDirectoryAbsolute());
              } else {
                mContext->SetSelectedObject(nullptr);
                selectedDir = entryAbsPath;
              }
            }
            if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
              renaming = RenameInfo{ .newName = entryAbsPath.stem().string(), .src = entryAbsPath };
            }
          }
        } else if (auto const asset{ mContext->GetResources().TryGetAssetAt(entryAbsPath) }) {
          if (renaming && equivalent(renaming->src, entryAbsPath)) {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##Rename", &renaming->newName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
              auto renamedAsset{ mContext->GetResources().UnregisterAsset(renaming->src) };

              auto const oldAssetMetaPath{ std::filesystem::path{ renaming->src } += mContext->GetAssetFileExtension() };
              auto const newAssetMetaPath{ renaming->src.parent_path() / renaming->newName += renaming->src.extension() += mContext->GetAssetFileExtension() };
              std::filesystem::rename(oldAssetMetaPath, newAssetMetaPath);

              auto const newAssetPath{ renaming->src.parent_path() / renaming->newName += renaming->src.extension() };
              std::filesystem::rename(renaming->src, newAssetPath);

              renamedAsset->SetName(newAssetPath.stem().string());
              mContext->GetResources().RegisterAsset(std::move(renamedAsset), newAssetPath);
              renaming.reset();
            }
            if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
              renaming.reset();
            }
          } else {
            auto isSelected{ asset == mContext->GetSelectedObject() };

            if (ImGui::Selectable(entryAbsPath.stem().string().c_str(), isSelected)) {
              mContext->SetSelectedObject(asset);
              selectedDir.reset();
            }

            if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
              renaming = RenameInfo{ .newName = entryAbsPath.stem().string(), .src = entryAbsPath };
            }
          }
        }
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}
}
