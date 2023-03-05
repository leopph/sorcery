#include "Serialization.hpp"
#include "Widgets.hpp"
#include "Material.hpp"
#include "Time.hpp"
#include "BinarySerializer.hpp"
#include "ModelImport.hpp"
#include "Asset.hpp"
#include "ObjectFactoryManager.hpp"
#include "MeshImporter.hpp"
#include "TextureImporter.hpp"
#include "Texture2D.hpp"

#include <TransformComponent.hpp>
#include <BehaviorComponent.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Entity.hpp>
#include <OnGui.hpp>
#include <Scene.hpp>
#include <Systems.hpp>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>

#include <YamlInclude.hpp>
#include <nfd.h>

#include <DirectXMath.h>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include "Mesh.hpp"


extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;


namespace leopph::editor {
class ResourceStorage {
	std::unordered_map<std::filesystem::path, std::shared_ptr<Object>> mData;

public:
	auto operator[](std::filesystem::path const& path) -> auto&& {
		return mData[absolute(path)];
	}

	auto find(std::filesystem::path const& path) const {
		return mData.find(path);
	}

	auto begin() const noexcept {
		return std::begin(mData);
	}

	auto end() const noexcept {
		return std::end(mData);
	}

	auto cbegin() const noexcept {
		return std::cbegin(mData);
	}

	auto cend() const noexcept {
		return std::cend(mData);
	}

	auto clear() noexcept -> decltype(auto) {
		mData.clear();
	}
};

namespace {
std::string_view constexpr RESOURCE_FILE_EXT{ ".leopphres" };

Object* gSelected{ nullptr };

auto EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) -> bool {
	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}


std::atomic gLoading{ false };

auto LoadAndBlockEditor(ImGuiIO& io, std::function<void()> const& fun) -> void {
	auto const oldFlags{ io.ConfigFlags };
	io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
	io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
	gLoading = true;
	fun();
	io.ConfigFlags = oldFlags;
	gLoading = false;
}

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

auto OpenProject(std::filesystem::path const& targetPath, ResourceStorage& resourceStorage, std::unique_ptr<Scene>& workingScene, std::filesystem::path& projPathAbs, std::filesystem::path const& assetDirRel, EditorObjectFactoryManager const& factoryManager) -> void {
	workingScene = std::make_unique<Scene>();
	resourceStorage.clear();
	projPathAbs = absolute(targetPath);

	/*struct AssetImportData {
		std::shared_ptr<leopph::Object> asset;
		std::vector<leopph::u8> metaContentBytes;
	};

	std::unordered_map<std::filesystem::path, AssetImportData> importData;

	for (auto const& entry : std::filesystem::recursive_directory_iterator{ projPathAbs / assetDirRel }) {
		if (entry.path().extension() == RESOURCE_FILE_EXT) {
			std::ifstream metaIn{ entry.path(), std::ios::binary };
			std::vector<leopph::u8> bytes;
			bytes.insert(std::begin(bytes), std::istreambuf_iterator<char>{ metaIn }, std::istreambuf_iterator<char>{});
			importData[std::filesystem::path{ entry.path() }.replace_extension()].metaContentBytes = std::move(bytes);
		}
		else {
			importData[entry.path()].asset = leopph::editor::LoadAsset(entry.path());
		}
	}

	for (auto& [path, data] : importData) {
		leopph::editor::AssignAssetMetaContents(*data.asset, data.metaContentBytes);
		resourceStorage[path] = data.asset;
	}
	TODO*/
}
}

auto DrawStartupScreen(ResourceStorage& resources, std::unique_ptr<Scene>& scene, std::filesystem::path& projDirAbs, std::filesystem::path const& assetDirRel, EditorObjectFactoryManager const& factoryManager) -> void {
	auto constexpr flags{ ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings };
	auto const viewport{ ImGui::GetMainViewport() };
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	if (ImGui::Begin("Open Project##OpenProjectWindow", nullptr, flags)) {
		auto const openProjectButtonLabel{ "Open Project##OpenProjectButton" };
		auto const windowSize{ ImGui::GetWindowSize() };
		auto const textSize{ ImGui::CalcTextSize(openProjectButtonLabel) };

		ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
		ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);

		if (ImGui::Button(openProjectButtonLabel)) {
			if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
				OpenProject(std::filesystem::path{ selectedPath }, resources, scene, projDirAbs, assetDirRel, factoryManager);
			}
		}
	}
	ImGui::End();
}

auto DrawMainMenuBar(ResourceStorage& resources, std::unique_ptr<Scene>& scene, std::filesystem::path& projDirAbs, std::filesystem::path const& assetDirRel, ImGuiIO& io, EditorObjectFactoryManager const& factoryManager, bool& showDemoWindow) -> void {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open Project")) {
				if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
					auto const LoadAndAssignProject = [selectedPath, &resources, &scene, &projDirAbs, &assetDirRel, &factoryManager] {
						OpenProject(std::filesystem::path{ selectedPath }, resources, scene, projDirAbs, assetDirRel, factoryManager);
						std::free(selectedPath);
					};

					std::thread loaderThread{ LoadAndBlockEditor, std::ref(io), LoadAndAssignProject };
					loaderThread.detach();
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("Entity")) {
				auto& entity{ scene->CreateEntity() };
				entity.CreateManagedObject();

				auto transform = std::make_unique<TransformComponent>();
				transform->CreateManagedObject();

				entity.AddComponent(std::move(transform));
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug")) {
			if (showDemoWindow) {
				if (ImGui::MenuItem("Hide Demo Window")) {
					showDemoWindow = false;
				}
			}
			else {
				if (ImGui::MenuItem("Show Demo Window")) {
					showDemoWindow = true;
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

auto DrawEntityHierarchyWindow(std::unique_ptr<Scene>& scene) -> void {
	if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
		auto constexpr baseFlags{ ImGuiTreeNodeFlags_OpenOnArrow };
		auto constexpr entityPayloadType{ "ENTITY" };

		if (ImGui::BeginDragDropTarget()) {
			if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
				static_cast<Entity*>(payload->Data)->GetTransform().SetParent(nullptr);
				ImGui::EndDragDropTarget();
			}
		}

		auto entities{ scene->GetEntities() };

		for (std::size_t i = 0; i < entities.size(); i++) {
			std::function<void(Entity&)> displayEntityRecursive;
			displayEntityRecursive = [&displayEntityRecursive, &entities, &scene](Entity& entity) -> void {
				ImGuiTreeNodeFlags nodeFlags{ baseFlags };

				if (entity.GetTransform().GetChildren().empty()) {
					nodeFlags |= ImGuiTreeNodeFlags_Leaf;
				}
				else {
					nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
				}

				if (gSelected && gSelected->GetGuid() == entity.GetGuid()) {
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				bool const nodeOpen{ ImGui::TreeNodeEx(entity.GetName().data(), nodeFlags) };

				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload(entityPayloadType, &entity, sizeof entity);
					ImGui::Text(entity.GetName().data());
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
						static_cast<Entity*>(payload->Data)->GetTransform().SetParent(&entity.GetTransform());
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					gSelected = &entity;
				}

				bool deleted{ false };

				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete")) {
						entity.GetScene().DestroyEntity(entity);
						entities = scene->GetEntities();
						gSelected = nullptr;
						deleted = true;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);

				if (nodeOpen) {
					if (!deleted) {
						for (std::size_t childIndex{ 0 }; childIndex < entity.GetTransform().GetChildren().size(); childIndex++) {
							ImGui::PushID(static_cast<int>(childIndex));
							displayEntityRecursive(*entity.GetTransform().GetChildren()[childIndex]->GetEntity());
							ImGui::PopID();
						}
					}
					ImGui::TreePop();
				}
			};

			if (!entities[i]->GetTransform().GetParent()) {
				ImGui::PushID(static_cast<int>(i));
				displayEntityRecursive(*entities[i]);
				ImGui::PopID();
			}
		}
	}
	ImGui::End();
}

auto DrawObjectPropertiesWindow(EditorObjectFactoryManager const& factoryManager) -> void {
	ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
		if (gSelected) {
			factoryManager.GetFor(gSelected->GetSerializationType()).OnGui(factoryManager, *gSelected);
		}
	}
	ImGui::End();
}

auto DrawGameViewWindow(bool const gameRunning) -> void {
	ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

	ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (gameRunning) {
		ImGui::SetNextWindowCollapsed(false);
		ImGui::SetNextWindowFocus();
	}

	if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
		ImGui::PopStyleVar();

		Extent2D<u32> constexpr resolutions[]{ { 960, 540 }, { 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 } };
		constexpr char const* resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
		static int selectedRes = 0;

		if (ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7)) {
			if (selectedRes != 0) {
				gRenderer.SetGameResolution(resolutions[selectedRes - 1]);
			}
		}

		auto const gameRes = gRenderer.GetGameResolution();
		auto const contentRegionSize = ImGui::GetContentRegionAvail();
		Extent2D const viewportRes{ static_cast<u32>(contentRegionSize.x), static_cast<u32>(contentRegionSize.y) };
		ImVec2 frameDisplaySize;

		if (selectedRes == 0) {
			if (viewportRes.width != gameRes.width || viewportRes.height != gameRes.height) {
				gRenderer.SetGameResolution(viewportRes);
			}

			frameDisplaySize = contentRegionSize;
		}
		else {
			f32 const scale = std::min(contentRegionSize.x / static_cast<f32>(gameRes.width), contentRegionSize.y / static_cast<f32>(gameRes.height));
			frameDisplaySize = ImVec2(static_cast<f32>(gameRes.width) * scale, static_cast<f32>(gameRes.height) * scale);
		}

		gRenderer.DrawGame();
		ImGui::Image(gRenderer.GetGameFrame(), frameDisplaySize);
	}
	else {
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

auto DrawSceneViewWindow(ImGuiIO const& io) -> void {
	ImVec2 static constexpr sceneViewportMinSize{ 480, 270 };

	ImGui::SetNextWindowSize(sceneViewportMinSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(sceneViewportMinSize, ImGui::GetMainViewport()->WorkSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse)) {
		ImGui::PopStyleVar();
		auto const [sceneWidth, sceneHeight]{ gRenderer.GetSceneResolution() };
		auto const contentRegionSize{ ImGui::GetContentRegionAvail() };

		if (Extent2D const viewportRes{ static_cast<u32>(contentRegionSize.x), static_cast<u32>(contentRegionSize.y) };
			viewportRes.width != sceneWidth || viewportRes.height != sceneHeight) {
			gRenderer.SetSceneResolution(viewportRes);
		}

		EditorCamera static editorCam{
			.position = Vector3{},
			.orientation = Quaternion{},
			.nearClip = 0.03f,
			.farClip = 10000.f,
			.fovVertRad = ToRadians(60),
		};

		static bool isMovingSceneCamera{ false };

		auto const wasMovingSceneCamera{ isMovingSceneCamera };
		isMovingSceneCamera = wasMovingSceneCamera ?
			                      ImGui::IsMouseDown(ImGuiMouseButton_Right) :
			                      ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

		if (!wasMovingSceneCamera && isMovingSceneCamera) {
			gWindow.LockCursor(gWindow.GetCursorPosition());
			gWindow.SetCursorHiding(true);
		}
		else if (wasMovingSceneCamera && !isMovingSceneCamera) {
			gWindow.UnlockCursor();
			gWindow.SetCursorHiding(false);
		}

		if (isMovingSceneCamera) {
			ImGui::SetWindowFocus();

			Vector3 posDelta{ 0, 0, 0 };
			if (GetKey(Key::W) || GetKey(Key::UpArrow)) {
				posDelta += Vector3::Forward();
			}
			if (GetKey(Key::A) || GetKey(Key::LeftArrow)) {
				posDelta += Vector3::Left();
			}
			if (GetKey(Key::D) || GetKey(Key::RightArrow)) {
				posDelta += Vector3::Right();
			}
			if (GetKey(Key::S) || GetKey(Key::DownArrow)) {
				posDelta += Vector3::Backward();
			}

			Normalize(posDelta);

			if (GetKey(Key::Shift)) {
				posDelta *= 2;
			}

			editorCam.position += editorCam.orientation.Rotate(posDelta) * get_frame_time() * 2;

			auto const [mouseX, mouseY]{ gWindow.GetMouseDelta() };
			auto constexpr sens{ 0.05f };

			editorCam.orientation = Quaternion{ Vector3::Up(), static_cast<f32>(mouseX) * sens } * editorCam.orientation;
			editorCam.orientation *= Quaternion{ Vector3::Right(), static_cast<f32>(mouseY) * sens };
		}

		gRenderer.DrawSceneView(editorCam);
		ImGui::Image(gRenderer.GetSceneFrame(), contentRegionSize);

		auto const editorCamViewMat{ Matrix4::LookAtLH(editorCam.position, editorCam.position + editorCam.orientation.Rotate(Vector3::Forward()), Vector3::Up()) };
		auto const editorCamProjMat{ Matrix4::PerspectiveAsymZLH(editorCam.fovVertRad, ImGui::GetWindowWidth() / ImGui::GetWindowHeight(), editorCam.nearClip, editorCam.farClip) };

		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetDrawlist();

		static bool showGrid{ true };

		if (GetKeyDown(Key::G)) {
			showGrid = !showGrid;
		}

		if (showGrid) {
			ImGuizmo::DrawGrid(editorCamViewMat.GetData(), editorCamProjMat.GetData(), Matrix4::Identity().GetData(), editorCam.farClip);
		}

		if (auto const selectedEntity{ dynamic_cast<Entity*>(gSelected) }; selectedEntity) {
			static auto op{ ImGuizmo::OPERATION::TRANSLATE };

			if (!io.WantTextInput && !isMovingSceneCamera) {
				if (GetKeyDown(Key::T)) {
					op = ImGuizmo::TRANSLATE;
				}
				if (GetKeyDown(Key::R)) {
					op = ImGuizmo::ROTATE;
				}
				if (GetKeyDown(Key::S)) {
					op = ImGuizmo::SCALE;
				}
				if (GetKeyDown(Key::F)) {
					editorCam.position = selectedEntity->GetTransform().GetWorldPosition() - Vector3::Forward() * 2;
					editorCam.orientation = selectedEntity->GetTransform().GetLocalRotation();
				}
			}

			if (Matrix4 modelMat{ selectedEntity->GetTransform().GetModelMatrix() }; Manipulate(editorCamViewMat.GetData(), editorCamProjMat.GetData(), op, ImGuizmo::MODE::LOCAL, modelMat.GetData())) {
				Vector3 pos, euler, scale;
				ImGuizmo::DecomposeMatrixToComponents(modelMat.GetData(), pos.GetData(), euler.GetData(), scale.GetData());
				selectedEntity->GetTransform().SetWorldPosition(pos);
				selectedEntity->GetTransform().SetWorldRotation(Quaternion::FromEulerAngles(euler));
				selectedEntity->GetTransform().SetWorldScale(scale);
			}
		}
	}
	else {
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

auto DrawProjectWindow(ResourceStorage& resources, std::filesystem::path const& projDirAbs, std::filesystem::path const& assetDirRel, MeshImporter const& meshImporter, TextureImporter const& texImporter, ImGuiIO& imGuiIo) -> void {
	if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoCollapse)) {
		if (ImGui::BeginTable("ProjectWindowMainTable", 2, ImGuiTableFlags_Resizable)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::filesystem::path static selectedProjSubDir{ projDirAbs };
			[&](this auto self, std::filesystem::path const& dir) -> void {
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
							self(entry.path());
						}
					}
					ImGui::TreePop();
				}
			}(projDirAbs);

			ImGui::TableSetColumnIndex(1);
			if (auto constexpr buttonSize{ 75 }; ImGui::BeginTable("ProjectWindowSubDirTable", static_cast<int>(ImGui::GetContentRegionAvail().x) / buttonSize)) {
				auto constexpr contextMenuId{ "ProjectWindowContextMenu" };

				if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					ImGui::OpenPopup(contextMenuId);
				}

				auto const createAssetMetaFile{
					[&projDirAbs, &assetDirRel](leopph::Object const& asset) {
						std::ofstream{ projDirAbs / assetDirRel / asset.GetName() += RESOURCE_FILE_EXT } << leopph::editor::GenerateAssetMetaFileContents(asset);
					}
				};

				if (ImGui::BeginPopup(contextMenuId)) {
					if (ImGui::BeginMenu("Create##CreateAssetMenu")) {
						auto const saveNewAsset{
							[&resources, &projDirAbs, &assetDirRel](std::shared_ptr<leopph::Object> asset) {
								auto const outPath{ IndexFileNameIfNeeded(projDirAbs / assetDirRel / asset->GetName() += RESOURCE_FILE_EXT) };
								asset->SetName(outPath.stem().string());

								static std::vector<leopph::u8> outBytes;
								asset->SerializeBinary(outBytes);

								std::ofstream out{ outPath, std::ios::binary };
								std::ranges::copy(std::as_const(outBytes), std::ostream_iterator<leopph::u8>{ out });
								outBytes.clear();

								gSelected = asset.get();

								resources[outPath] = std::move(asset);
							}
						};

						if (ImGui::MenuItem("Material##CreateMaterialAsset")) {
							auto newMtl{ std::make_shared<leopph::Material>() };
							newMtl->SetName("New Material");
							saveNewAsset(std::move(newMtl));
							createAssetMetaFile(*newMtl);
						}

						if (ImGui::MenuItem("Scene##CreateSceneAsset")) {
							auto newScene{ std::make_shared<leopph::Scene>() };
							newScene->SetName("New Scene");
							saveNewAsset(std::move(newScene));
							createAssetMetaFile(*newScene);
						}

						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Import##ImportAssetMenu")) {
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

						auto const copyAssetFileToProjDir{
							[&projDirAbs, &assetDirRel](std::filesystem::path const& srcPath) -> std::filesystem::path {
								auto const srcPathAbs{ absolute(srcPath) };
								auto dstPath{ IndexFileNameIfNeeded(projDirAbs / assetDirRel / srcPath.filename()) };
								copy(srcPath, dstPath);
								return dstPath;
							}
						};

						auto const saveNewAsset{
							[&createAssetMetaFile, &resources](std::shared_ptr<leopph::Object> asset, std::filesystem::path const& dstPath) {
								asset->SetName(dstPath.filename().string());
								createAssetMetaFile(*asset);
								resources[dstPath] = std::move(asset);
							}
						};

						if (ImGui::MenuItem("Mesh##ImportMeshAssetMenuItem")) {
							if (std::filesystem::path path; openFileDialog(meshImporter.GetSupportedExtensions().c_str(), nullptr, path)) {
								std::thread loaderThread{
									LoadAndBlockEditor, std::ref(imGuiIo), [&copyAssetFileToProjDir, &saveNewAsset, &meshImporter, path] {
										auto const assetPath{ copyAssetFileToProjDir(path) };
										saveNewAsset(std::make_shared<leopph::Mesh>(std::move(meshImporter.Import(assetPath))), assetPath);
									}
								};
								loaderThread.detach();
							}

							ImGui::CloseCurrentPopup();
						}

						if (ImGui::MenuItem("Texture##ImportTextureAssetMenuItem")) {
							if (std::filesystem::path path; openFileDialog(texImporter.GetSupportedExtensions().c_str(), nullptr, path)) {
								std::thread loaderThread{
									LoadAndBlockEditor, std::ref(imGuiIo), [&copyAssetFileToProjDir, &saveNewAsset, &texImporter, path] {
										auto const assetPath{ copyAssetFileToProjDir(path) };
										saveNewAsset(std::make_shared<leopph::Texture2D>(std::move(texImporter.Import(assetPath))), assetPath);
									}
								};
								loaderThread.detach();
							}

							ImGui::CloseCurrentPopup();
						}

						ImGui::EndMenu();
					}

					ImGui::EndPopup();
				}

				for (auto const& entry : std::filesystem::directory_iterator{ selectedProjSubDir }) {
					ImGui::TableNextColumn();
					auto const& entryPath{ entry.path() };
					auto const entryStemStr{ entryPath.stem().string() };
					if (auto const resIt{ resources.find(absolute(entryPath)) };
						(is_directory(entryPath) || resIt != std::end(resources)) &&
						ImGui::Button(entryStemStr.c_str(), { buttonSize, buttonSize })) {
						if (is_directory(entryPath)) {
							selectedProjSubDir = entryPath;
						}
						else {
							gSelected = resources.find(absolute(entryPath))->second.get();
						}
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndTable();
	}
	ImGui::End();
}
}


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, [[maybe_unused]] _In_ wchar_t*, [[maybe_unused]] _In_ int) -> int {
	try {
		leopph::editor::EditorObjectFactoryManager factoryManager;
		factoryManager.Register<leopph::Entity>();
		factoryManager.Register<leopph::TransformComponent>();
		factoryManager.Register<leopph::CameraComponent>();
		factoryManager.Register<leopph::BehaviorComponent>();
		factoryManager.Register<leopph::CubeModelComponent>();
		factoryManager.Register<leopph::LightComponent>();
		factoryManager.Register<leopph::Material>();
		factoryManager.Register<leopph::Mesh>();
		factoryManager.Register<leopph::Texture2D>();

		leopph::editor::MeshImporter meshImporter;
		leopph::editor::TextureImporter texImporter;

		leopph::gWindow.StartUp();
		leopph::gRenderer.StartUp();
		leopph::gManagedRuntime.StartUp();

		leopph::gWindow.SetBorderless(false);
		leopph::gWindow.SetWindowedClientAreaSize({ 1280, 720 });
		leopph::gWindow.SetIgnoreManagedRequests(true);

		leopph::gRenderer.SetGameResolution({ 960, 540 });
		leopph::gRenderer.SetSyncInterval(1);

		ImGui::CreateContext();
		auto& imGuiIo = ImGui::GetIO();
		auto const iniFilePath{ std::filesystem::path{ leopph::GetExecutablePath() }.remove_filename() /= "editorconfig.ini" };
		auto const iniFilePathStr{ leopph::WideToUtf8(iniFilePath.c_str()) };
		imGuiIo.IniFilename = iniFilePathStr.c_str();
		imGuiIo.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		leopph::SetImGuiContext(ImGui::GetCurrentContext());

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(leopph::gWindow.GetHandle());
		ImGui_ImplDX11_Init(leopph::gRenderer.GetDevice(), leopph::gRenderer.GetImmediateContext());

		leopph::gWindow.SetEventHook(leopph::editor::EditorImGuiEventHook);

		bool runGame{ false };
		bool showDemoWindow{ false };

		leopph::editor::ResourceStorage resources;
		auto scene{ std::make_unique<leopph::Scene>() };

		std::filesystem::path projDirAbs;
		std::filesystem::path const assetDirRel{ "Assets" };
		std::filesystem::path const cacheDirRel{ "Cache" };

		leopph::init_time();

		while (!leopph::gWindow.IsQuitSignaled()) {
			leopph::gWindow.ProcessEvents();

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			if (projDirAbs.empty()) {
				DrawStartupScreen(resources, scene, projDirAbs, assetDirRel, factoryManager);
			}
			else {
				if (runGame) {
					leopph::init_behaviors();
					leopph::tick_behaviors();
					leopph::tack_behaviors();

					if (GetKeyDown(leopph::Key::Escape)) {
						runGame = false;
						leopph::gWindow.SetEventHook(leopph::editor::EditorImGuiEventHook);
						leopph::gWindow.UnlockCursor();
						leopph::gWindow.SetCursorHiding(false);
						leopph::gRenderer.SetSyncInterval(1);
						scene->Load(factoryManager);
						leopph::editor::gSelected = nullptr;
					}
				}
				else {
					if (GetKeyDown(leopph::Key::F5)) {
						runGame = true;
						leopph::gWindow.SetEventHook({});
						leopph::gRenderer.SetSyncInterval(0);
						scene->Save();
					}
				}

				ImGui::DockSpaceOverViewport();

				if (leopph::editor::gLoading) {
					ImGui::SetNextWindowPos(ImVec2(imGuiIo.DisplaySize.x * 0.5f, imGuiIo.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
					if (ImGui::Begin("LoadingIndicator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
						leopph::editor::DrawSpinner("##spinner", 15, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
					}
					ImGui::End();
				}

				if (showDemoWindow) {
					ImGui::ShowDemoWindow();
				}

				DrawMainMenuBar(resources, scene, projDirAbs, assetDirRel, imGuiIo, factoryManager, showDemoWindow);

				leopph::editor::DrawEntityHierarchyWindow(scene);
				DrawObjectPropertiesWindow(factoryManager);
				leopph::editor::DrawGameViewWindow(runGame);
				leopph::editor::DrawSceneViewWindow(imGuiIo);
				DrawProjectWindow(resources, projDirAbs, assetDirRel, meshImporter, texImporter, imGuiIo);
			}

			ImGui::Render();

			leopph::gRenderer.BindAndClearSwapChain();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			leopph::gRenderer.Present();

			leopph::measure_time();
		}

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
	catch (std::exception const& ex) {
		leopph::DisplayError(ex.what());
	}

	leopph::gManagedRuntime.ShutDown();
	leopph::gRenderer.ShutDown();
	leopph::gWindow.ShutDown();
	return 0;
}
