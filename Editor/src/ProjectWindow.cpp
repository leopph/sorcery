#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ProjectWindow.hpp"

#include <nfd.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <misc/cpp/imgui_stdlib.h>

#include <functional>


namespace leopph::editor {
namespace {
auto IndexFileNameIfNeeded(std::filesystem::path const& filePathAbsolute) -> std::filesystem::path {
	std::string const originalStem{ filePathAbsolute.stem().string() };
	std::filesystem::path const ext{ filePathAbsolute.extension() };
	std::filesystem::path const parentDir{ filePathAbsolute.parent_path() };

	std::string currentStem{ originalStem };
	std::size_t fileNameIndex{ 1 };

	while (true) {
		bool isUsed{ false };
		for (auto const& entry : std::filesystem::directory_iterator{ parentDir }) {
			if (entry.path().stem() == currentStem) {
				currentStem = originalStem;
				currentStem += " ";
				currentStem += std::to_string(fileNameIndex);
				++fileNameIndex;
				isUsed = true;
				break;
			}
		}

		if (!isUsed) {
			break;
		}
	}

	return (parentDir / currentStem).replace_extension(ext);
}


auto DrawSideFolderView(std::filesystem::path const& dir, std::filesystem::path& selectedProjSubDir) -> void {
	std::filesystem::directory_iterator const it{ dir };
	ImGuiTreeNodeFlags treeNodeFlags{ ImGuiTreeNodeFlags_OpenOnArrow };

	if (begin(it) == end(it)) {
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}
	else {
		treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
	}
	if (selectedProjSubDir == dir) {
		treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (ImGui::TreeNodeEx(dir.stem().string().c_str(), treeNodeFlags)) {
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			selectedProjSubDir = dir;
		}

		for (auto const& entry : it) {
			if (is_directory(entry)) {
				DrawSideFolderView(entry.path(), selectedProjSubDir);
			}
		}
		ImGui::TreePop();
	}
}
}


auto DrawProjectWindow(Context& context) -> void {
	if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoCollapse)) {
		if (ImGui::BeginTable("ProjectWindowMainTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			std::filesystem::path static selectedProjSubDir{ context.GetAssetDirectoryAbsolute() };
			DrawSideFolderView(context.GetAssetDirectoryAbsolute(), selectedProjSubDir);

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
				[&context](Importer& importer, std::filesystem::path const& srcPathAbs) {
					context.ExecuteInBusyEditor([&context, &importer, srcPathAbs] {
						auto const dstPath{
							equivalent(srcPathAbs.parent_path(), selectedProjSubDir) ?
								srcPathAbs :
								IndexFileNameIfNeeded(context.GetAssetDirectoryAbsolute() / srcPathAbs.filename())
						};

						if (!exists(dstPath) || !equivalent(dstPath, srcPathAbs)) {
							copy_file(srcPathAbs, dstPath);
						}

						auto const guid{ Guid::Generate() };

						Importer::InputImportInfo const info{
							.src = dstPath,
							.guid = guid
						};

						if (auto const asset{ importer.Import(info, context.GetCacheDirectoryAbsolute()) }) {
							asset->SetName(dstPath.stem().string());
							asset->SetGuid(guid);

							context.GetResources().RegisterAsset(std::unique_ptr<Object>{ asset }, dstPath);
							context.CreateMetaFileForRegisteredAsset(*asset);
						}
						else {
							throw std::runtime_error{ std::format("Failed to import asset at {}.", srcPathAbs.string()) };
						}
					});
				}
			};

			auto const importAsset{
				[&context, openFileDialog, importConcreteAsset](Object::Type const targetAssetType) {
					auto& importer{ context.GetFactoryManager().GetFor(targetAssetType).GetImporter() };
					if (std::filesystem::path path; openFileDialog(importer.GetSupportedExtensions().c_str(), nullptr, path)) {
						importConcreteAsset(importer, absolute(path));
					}
				}
			};

			auto openCubemapImportModal{ false };

			if (ImGui::BeginPopup(contextMenuId)) {
				if (ImGui::BeginMenu("Create##CreateAssetMenu")) {
					if (ImGui::MenuItem("Material##CreateMaterialAsset")) {
						auto const dstPath{ IndexFileNameIfNeeded(context.GetAssetDirectoryAbsolute() / "New Material.mtl") };

						auto const mtl{ new Material{} };
						mtl->SetName(dstPath.stem().string());

						context.GetResources().RegisterAsset(std::unique_ptr<Object>{ mtl }, dstPath);
						context.SaveRegisteredNativeAsset(*mtl);
						context.CreateMetaFileForRegisteredAsset(*mtl);
					}

					if (ImGui::MenuItem("Scene##CreateSceneAsset")) {
						auto const dstPath{ IndexFileNameIfNeeded(context.GetAssetDirectoryAbsolute() / "New Scene.scene") };

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
								openFileDialog(context.GetFactoryManager().GetFor<Cubemap>().GetImporter().GetSupportedExtensions().c_str(), nullptr, paths[i]);
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
					}
					else if (importTypeIdx == 1) {
						bool rdyToImport{ true };

						if (cubeMapcombinedFileName.empty()) {
							rdyToImport = false;
							MessageBoxW(nullptr, L"Please enter the name of the combined image file.", L"Error", MB_ICONERROR);
						}

						auto const dstPath{ selectedProjSubDir / std::filesystem::path{ cubeMapcombinedFileName } += ".png" };

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
								}
								else {
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
								}
								else {
									importConcreteAsset(context.GetFactoryManager().GetFor<Cubemap>().GetImporter(), dstPath);
									ImGui::CloseCurrentPopup();
								}
							}
						}
					}
				}

				ImGui::EndPopup();
			}

			std::filesystem::path static renameAssetPath;

			for (auto const& entry : std::filesystem::directory_iterator{ selectedProjSubDir }) {
				auto const entryPathAbs{ absolute(entry.path()) };

				if (auto const asset{ context.GetResources().TryGetAssetAt(entryPathAbs) }; asset || is_directory(entryPathAbs)) {
					if (!renameAssetPath.empty() && equivalent(renameAssetPath, entryPathAbs)) {
						std::string input{ asset->GetName() };

						ImGui::SetKeyboardFocusHere(0);

						if (ImGui::InputText("###RenameAsset", &input, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
							auto renamedAsset{ context.GetResources().UnregisterAsset(renameAssetPath) };

							auto const oldAssetMetaPath{ std::filesystem::path{ renameAssetPath } += context.GetAssetFileExtension() };
							auto const newAssetMetaPath{ renameAssetPath.parent_path() / input += renameAssetPath.extension() += context.GetAssetFileExtension() };
							rename(oldAssetMetaPath, newAssetMetaPath);

							auto const newAssetPath{ renameAssetPath.parent_path() / input += renameAssetPath.extension() };
							rename(renameAssetPath, newAssetPath);

							renamedAsset->SetName(newAssetPath.stem().string());
							context.GetResources().RegisterAsset(std::move(renamedAsset), newAssetPath);
							renameAssetPath.clear();
						}

						if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) {
							renameAssetPath.clear();
						}
					}
					else {
						auto const selected{ context.GetResources().TryGetAssetAt(entryPathAbs) == context.GetSelectedObject() };

						if (ImGui::Selectable(entryPathAbs.stem().string().c_str(), selected)) {
							if (is_directory(entryPathAbs)) {
								selectedProjSubDir = entryPathAbs;
							}
							else {
								context.SetSelectedObject(asset);
							}
						}

						if ((ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 2) || (selected && ImGui::IsKeyPressed(ImGuiKey_F2, false))) {
							renameAssetPath = entryPathAbs;
						}
					}
				}
			}
		}
		ImGui::EndTable();
	}
	ImGui::End();
}
}
