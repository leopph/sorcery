#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ProjectWindow.hpp"
#include "Util.hpp"

#include <nfd.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <misc/cpp/imgui_stdlib.h>

#include <functional>
#include <optional>


namespace leopph::editor {
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


auto DrawProjectWindow(Context& context) -> void {
  if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoCollapse)) {
    if (ImGui::BeginTable("ProjectWindowMainTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit)) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      std::filesystem::path static currentRelativeDir{ "" };

      if (!exists(context.GetAssetDirectoryAbsolute() / currentRelativeDir)) {
        currentRelativeDir = "";
      }

      DrawSideFolderView(context.GetAssetDirectoryAbsolute(), currentRelativeDir);

      auto const currentAbsoluteDir{ context.GetAssetDirectoryAbsolute() / currentRelativeDir };

      ImGui::TableSetColumnIndex(1);

      auto constexpr contextMenuId{ "ProjectWindowContextMenu" };

      if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(contextMenuId);
      }

      auto const openFileDialog{
        [](char const* filters, char const* defaultPath, std::filesystem::path& out) -> bool {
          if (nfdchar_t* selectedPath{ nullptr }; NFD_OpenDialog(filters, defaultPath, &selectedPath) == NFD_OKAY) {
            out = selectedPath;
            std::free(selectedPath);
            return true;
          }
          return false;
        }
      };

      auto const importConcreteAsset{
        [&context, currentAbsoluteDir](AssetLoader& assetLoader, std::filesystem::path const& srcPathAbs) {
          context.ExecuteInBusyEditor([&context, &assetLoader, srcPathAbs, currentAbsoluteDir] {
            auto const dstPath{
              equivalent(srcPathAbs.parent_path(), currentAbsoluteDir) ?
                srcPathAbs :
                GenerateUniquePath(currentAbsoluteDir / srcPathAbs.filename())
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
      };

      auto const importAsset{
        [&context, openFileDialog, importConcreteAsset](Object::Type const targetAssetType) {
          auto& loader{ context.GetFactoryManager().GetFor(targetAssetType).GetLoader() };

          if (std::filesystem::path path; openFileDialog(Join(loader.GetSupportedExtensions(), ",").c_str(), nullptr, path)) {
            importConcreteAsset(loader, absolute(path));
          }
        }
      };

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
            auto const newFolderPath{ GenerateUniquePath(currentAbsoluteDir / "New Folder") };
            create_directory(newFolderPath);
            selectedDir = newFolderPath;
            context.SetSelectedObject(nullptr);
            renaming = RenameInfo{ .newName = newFolderPath.stem().string(), .src = newFolderPath };
          }

          if (ImGui::MenuItem("Material")) {
            auto const dst{ GenerateUniquePath(currentAbsoluteDir / "New Material.mtl") };

            auto const mtl{ new Material{} };
            mtl->SetName(dst.stem().string());

            context.GetResources().RegisterAsset(std::unique_ptr<Object>{ mtl }, dst);
            context.SaveRegisteredNativeAsset(*mtl);
            context.CreateMetaFileForRegisteredAsset(*mtl);
          }

          if (ImGui::MenuItem("Scene##CreateSceneAsset")) {
            auto const dstPath{ GenerateUniquePath(currentAbsoluteDir / "New Scene.scene") };

            auto const scene{ new Scene{} };
            scene->SetName(dstPath.stem().string());

            context.GetResources().RegisterAsset(std::unique_ptr<Object>{ scene }, dstPath);
            context.SaveRegisteredNativeAsset(*scene);
            context.CreateMetaFileForRegisteredAsset(*scene);
          }

          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Import##ImportAssetMenu")) {
          if (ImGui::MenuItem("Mesh##ImportMeshAssetMenuItem")) {
            importAsset(Object::Type::Mesh);
            ImGui::CloseCurrentPopup();
          }

          if (ImGui::MenuItem("2D Texture##Import2DTextureAssetMenuItem")) {
            importAsset(Object::Type::Texture2D);
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
          [&context, openFileDialog]<int ElemCount>(std::span<char const* const, ElemCount> labels, std::span<std::filesystem::path, ElemCount> paths) {
            for (int i = 0; i < ElemCount; i++) {
              ImGui::PushID(i);
              ImGui::TableNextColumn();
              ImGui::Text("%s", labels[i]);
              ImGui::TableNextColumn();
              if (ImGui::Button("Select")) {
                openFileDialog(Join(context.GetFactoryManager().GetFor<Cubemap>().GetLoader().GetSupportedExtensions(), ",").c_str(), nullptr, paths[i]);
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
            importAsset(Object::Type::Cubemap);
            ImGui::CloseCurrentPopup();
          } else if (importTypeIdx == 1) {
            bool rdyToImport{ true };

            if (cubeMapcombinedFileName.empty()) {
              rdyToImport = false;
              MessageBoxW(nullptr, L"Please enter the name of the combined image file.", L"Error", MB_ICONERROR);
            }

            auto const dstPath{ currentAbsoluteDir / std::filesystem::path{ cubeMapcombinedFileName } += ".png" };

            if (exists(dstPath)) {
              rdyToImport = false;
              MessageBoxW(nullptr, L"The filename is already in use.", L"Error", MB_ICONERROR);
            }

            for (auto& path : facePaths) {
              if (path.empty()) {
                rdyToImport = false;
                MessageBoxW(nullptr, L"Not all faces of the cubemap are selected. Please select all of them.", L"Error", MB_ICONERROR);
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
                    MessageBoxW(nullptr, std::format(L"Image at {} cannot be used as a cubemap face because it is not square.", facePaths[i].wstring()).c_str(), L"Error", MB_ICONERROR);
                    stbi_image_free(data);
                    break;
                  }

                  if (!IsPowerOfTwo(width)) {
                    importSuccess = false;
                    MessageBoxW(nullptr, std::format(L"Image at {} cannot be used as a cubemap face because its dimensions are not powers of two.", facePaths[i].wstring()).c_str(), L"Error", MB_ICONERROR);
                    stbi_image_free(data);
                    break;
                  }

                  if (i > 0 && static_cast<int>(loadedImages[i - 1].get_width()) != width) {
                    importSuccess = false;
                    MessageBoxW(nullptr, std::format(L"The image dimensions at {} don't match the dimensions of the previous images.", facePaths[i].wstring()).c_str(), L"Error", MB_ICONERROR);
                    stbi_image_free(data);
                    break;
                  }

                  loadedImages[i] = Image{ static_cast<u32>(width), static_cast<u32>(height), 4, std::unique_ptr<u8[]>{ data } };
                } else {
                  importSuccess = false;
                  MessageBoxW(nullptr, std::format(L"Failed to load image file at {}.", facePaths[i].wstring()).c_str(), L"Error", MB_ICONERROR);
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
                  MessageBoxW(nullptr, L"Failed to write combined image.", L"Error", MB_ICONERROR);
                } else {
                  importConcreteAsset(context.GetFactoryManager().GetFor<Cubemap>().GetLoader(), dstPath);
                  ImGui::CloseCurrentPopup();
                }
              }
            }
          }
        }

        ImGui::EndPopup();
      }

      for (auto const& entry : std::filesystem::directory_iterator{ currentAbsoluteDir }) {
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
                context.SetSelectedObject(nullptr);
                selectedDir.reset();
                currentRelativeDir = relative(entryAbsPath, context.GetAssetDirectoryAbsolute());
              } else {
                context.SetSelectedObject(nullptr);
                selectedDir = entryAbsPath;
              }
            }
            if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 3) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
              renaming = RenameInfo{ .newName = entryAbsPath.stem().string(), .src = entryAbsPath };
            }
          }
        } else if (auto const asset{ context.GetResources().TryGetAssetAt(entryAbsPath) }) {
          if (renaming && equivalent(renaming->src, entryAbsPath)) {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##Rename", &renaming->newName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
              auto renamedAsset{ context.GetResources().UnregisterAsset(renaming->src) };

              auto const oldAssetMetaPath{ std::filesystem::path{ renaming->src } += context.GetAssetFileExtension() };
              auto const newAssetMetaPath{ renaming->src.parent_path() / renaming->newName += renaming->src.extension() += context.GetAssetFileExtension() };
              std::filesystem::rename(oldAssetMetaPath, newAssetMetaPath);

              auto const newAssetPath{ renaming->src.parent_path() / renaming->newName += renaming->src.extension() };
              std::filesystem::rename(renaming->src, newAssetPath);

              renamedAsset->SetName(newAssetPath.stem().string());
              context.GetResources().RegisterAsset(std::move(renamedAsset), newAssetPath);
              renaming.reset();
            }
            if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
              renaming.reset();
            }
          } else {
            auto isSelected{ asset == context.GetSelectedObject() };

            if (ImGui::Selectable(entryAbsPath.stem().string().c_str(), isSelected)) {
              context.SetSelectedObject(asset);
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
