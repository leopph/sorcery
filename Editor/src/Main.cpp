#include "Serialization.hpp"
#include "Widgets.hpp"
#include "Material.hpp"
#include "Time.hpp"
#include "BinarySerializer.hpp"
#include "ModelImport.hpp"
#include "Asset.hpp"
#include "ObjectFactoryManager.hpp"

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


class ResourceStorage {
	std::unordered_map<std::filesystem::path, std::shared_ptr<leopph::Object>> mData;

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

	decltype(auto) clear() noexcept {
		mData.clear();
	}
};


namespace {
std::string_view constexpr RESOURCE_FILE_EXT{ ".leopphres" };

leopph::Object* gSelected{ nullptr };

auto EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) -> bool {
	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}


std::atomic<bool> gLoading{ false };

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

auto OpenProject(std::filesystem::path const& targetPath, ResourceStorage& resourceStorage, std::unique_ptr<leopph::Scene>& workingScene, std::filesystem::path& projPathAbs, std::filesystem::path const& assetDirRel) -> void {
	workingScene = std::make_unique<leopph::Scene>();
	resourceStorage.clear();
	projPathAbs = absolute(targetPath);

	struct AssetImportData {
		std::shared_ptr<leopph::Object> asset;
		std::vector<leopph::u8> metaContentBytes;
	};

	std::unordered_map<std::filesystem::path, AssetImportData> importData;

	for (auto const& entry : std::filesystem::recursive_directory_iterator{ projPathAbs / assetDirRel }) {
		if (entry.path().extension() == ".leopphasset") {
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
}
}


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, [[maybe_unused]] _In_ wchar_t*, [[maybe_unused]] _In_ int) -> int {
	try {
		leopph::ObjectFactory objectFactory;
		objectFactory.Register<leopph::Entity>();
		objectFactory.Register<leopph::TransformComponent>();
		objectFactory.Register<leopph::CameraComponent>();
		objectFactory.Register<leopph::BehaviorComponent>();
		objectFactory.Register<leopph::CubeModelComponent>();
		objectFactory.Register<leopph::LightComponent>();
		objectFactory.Register<leopph::Material>();
		objectFactory.Register<leopph::Mesh>();

		leopph::editor::ObjectFactoryManager objectFactoryManager;
		objectFactoryManager.Register<leopph::Entity>();
		objectFactoryManager.Register<leopph::TransformComponent>();
		objectFactoryManager.Register<leopph::CameraComponent>();
		objectFactoryManager.Register<leopph::BehaviorComponent>();
		objectFactoryManager.Register<leopph::CubeModelComponent>();
		objectFactoryManager.Register<leopph::LightComponent>();
		objectFactoryManager.Register<leopph::Material>();
		objectFactoryManager.Register<leopph::Mesh>();

		leopph::gWindow.StartUp();
		leopph::gRenderer.StartUp();
		leopph::gManagedRuntime.StartUp();

		leopph::gWindow.SetBorderless(false);
		leopph::gWindow.SetWindowedClientAreaSize({ 1280, 720 });
		leopph::gWindow.SetIgnoreManagedRequests(true);

		leopph::gRenderer.SetGameResolution({ 960, 540 });
		leopph::gRenderer.SetSyncInterval(1);

		ImGui::CreateContext();
		auto& io = ImGui::GetIO();
		auto const iniFilePath{ std::filesystem::path{ leopph::GetExecutablePath() }.remove_filename() /= "editorconfig.ini" };
		auto const iniFilePathStr{ leopph::WideToUtf8(iniFilePath.c_str()) };
		io.IniFilename = iniFilePathStr.c_str();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		leopph::SetImGuiContext(ImGui::GetCurrentContext());

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(leopph::gWindow.GetHandle());
		ImGui_ImplDX11_Init(leopph::gRenderer.GetDevice(), leopph::gRenderer.GetImmediateContext());

		leopph::gWindow.SetEventHook(EditorImGuiEventHook);

		bool runGame{ false };
		bool showDemoWindow{ false };

		ResourceStorage resources;
		auto scene{ std::make_unique<leopph::Scene>() };

		std::filesystem::path projDirAbs;
		std::filesystem::path const assetDirRel{ "Assets" };

		leopph::init_time();

		while (!leopph::gWindow.IsQuitSignaled()) {
			leopph::gWindow.ProcessEvents();

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			if (projDirAbs.empty()) {
				auto flags{ ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings };
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
							OpenProject(std::filesystem::path{ selectedPath }, resources, scene, projDirAbs, assetDirRel);
						}
					}
				}
				ImGui::End();
			}
			else {
				if (runGame) {
					leopph::init_behaviors();
					leopph::tick_behaviors();
					leopph::tack_behaviors();

					if (leopph::GetKeyDown(leopph::Key::Escape)) {
						runGame = false;
						leopph::gWindow.SetEventHook(EditorImGuiEventHook);
						leopph::gWindow.UnlockCursor();
						leopph::gWindow.SetCursorHiding(false);
						leopph::gRenderer.SetSyncInterval(1);
						scene->Load(objectFactory);
						gSelected = nullptr;
					}
				}
				else {
					if (leopph::GetKeyDown(leopph::Key::F5)) {
						runGame = true;
						leopph::gWindow.SetEventHook({});
						leopph::gRenderer.SetSyncInterval(0);
						scene->Save();
					}
				}

				ImGui::DockSpaceOverViewport();

				if (gLoading) {
					ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
					if (ImGui::Begin("LoadingIndicator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
						leopph::editor::DrawSpinner("##spinner", 15, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
					}
					ImGui::End();
				}

				if (showDemoWindow) {
					ImGui::ShowDemoWindow();
				}

				if (ImGui::BeginMainMenuBar()) {
					if (ImGui::BeginMenu("File")) {
						if (ImGui::MenuItem("Open Project")) {
							if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
								auto const LoadAndAssignProject = [selectedPath, &resources, &scene, &projDirAbs, &assetDirRel] {
									OpenProject(std::filesystem::path{ selectedPath }, resources, scene, projDirAbs, assetDirRel);
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

							auto transform = std::make_unique<leopph::TransformComponent>();
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

				auto entities{ scene->GetEntities() };


				if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
					auto constexpr baseFlags{ ImGuiTreeNodeFlags_OpenOnArrow };
					auto constexpr entityPayloadType{ "ENTITY" };

					if (ImGui::BeginDragDropTarget()) {
						if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
							static_cast<leopph::Entity*>(payload->Data)->GetTransform().SetParent(nullptr);
							ImGui::EndDragDropTarget();
						}
					}

					for (std::size_t i = 0; i < entities.size(); i++) {
						std::function<void(leopph::Entity&)> displayEntityRecursive;
						displayEntityRecursive = [&displayEntityRecursive, &entities, &scene](leopph::Entity& entity) -> void {
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
									static_cast<leopph::Entity*>(payload->Data)->GetTransform().SetParent(&entity.GetTransform());
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

				ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

				if (ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
					if (gSelected) {
						objectFactoryManager.GetWrapperFor(gSelected->GetSerializationType()).OnGui(objectFactoryManager, *gSelected);
					}
				}
				ImGui::End();

				ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

				ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

				if (runGame) {
					ImGui::SetNextWindowCollapsed(false);
					ImGui::SetNextWindowFocus();
				}

				if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
					ImGui::PopStyleVar();

					leopph::Extent2D<leopph::u32> constexpr resolutions[]{ { 960, 540 }, { 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 } };
					constexpr char const* resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
					static int selectedRes = 0;

					if (ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7)) {
						if (selectedRes != 0) {
							leopph::gRenderer.SetGameResolution(resolutions[selectedRes - 1]);
						}
					}

					auto const gameRes = leopph::gRenderer.GetGameResolution();
					auto const contentRegionSize = ImGui::GetContentRegionAvail();
					leopph::Extent2D<leopph::u32> const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };
					ImVec2 frameDisplaySize;

					if (selectedRes == 0) {
						if (viewportRes.width != gameRes.width || viewportRes.height != gameRes.height) {
							leopph::gRenderer.SetGameResolution(viewportRes);
						}

						frameDisplaySize = contentRegionSize;
					}
					else {
						leopph::f32 const scale = std::min(contentRegionSize.x / static_cast<leopph::f32>(gameRes.width), contentRegionSize.y / static_cast<leopph::f32>(gameRes.height));
						frameDisplaySize = ImVec2(static_cast<leopph::f32>(gameRes.width) * scale, static_cast<leopph::f32>(gameRes.height) * scale);
					}

					leopph::gRenderer.DrawGame();
					ImGui::Image(leopph::gRenderer.GetGameFrame(), frameDisplaySize);
				}
				else {
					ImGui::PopStyleVar();
				}
				ImGui::End();

				ImVec2 static constexpr sceneViewportMinSize{ 480, 270 };

				ImGui::SetNextWindowSize(sceneViewportMinSize, ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSizeConstraints(sceneViewportMinSize, ImGui::GetMainViewport()->WorkSize);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

				if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse)) {
					ImGui::PopStyleVar();
					auto const [sceneWidth, sceneHeight]{ leopph::gRenderer.GetSceneResolution() };
					auto const contentRegionSize{ ImGui::GetContentRegionAvail() };

					if (leopph::Extent2D const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };
						viewportRes.width != sceneWidth || viewportRes.height != sceneHeight) {
						leopph::gRenderer.SetSceneResolution(viewportRes);
					}

					leopph::EditorCamera static editorCam{
						.position = leopph::Vector3{},
						.orientation = leopph::Quaternion{},
						.nearClip = 0.03f,
						.farClip = 10000.f,
						.fovVertRad = leopph::ToRadians(60),
					};

					static bool isMovingSceneCamera{ false };

					auto const wasMovingSceneCamera{ isMovingSceneCamera };
					isMovingSceneCamera = wasMovingSceneCamera ?
						                      ImGui::IsMouseDown(ImGuiMouseButton_Right) :
						                      ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

					if (!wasMovingSceneCamera && isMovingSceneCamera) {
						leopph::gWindow.LockCursor(leopph::gWindow.GetCursorPosition());
						leopph::gWindow.SetCursorHiding(true);
					}
					else if (wasMovingSceneCamera && !isMovingSceneCamera) {
						leopph::gWindow.UnlockCursor();
						leopph::gWindow.SetCursorHiding(false);
					}

					if (isMovingSceneCamera) {
						ImGui::SetWindowFocus();

						leopph::Vector3 posDelta{ 0, 0, 0 };
						if (GetKey(leopph::Key::W) || GetKey(leopph::Key::UpArrow)) {
							posDelta += leopph::Vector3::Forward();
						}
						if (GetKey(leopph::Key::A) || GetKey(leopph::Key::LeftArrow)) {
							posDelta += leopph::Vector3::Left();
						}
						if (GetKey(leopph::Key::D) || GetKey(leopph::Key::RightArrow)) {
							posDelta += leopph::Vector3::Right();
						}
						if (GetKey(leopph::Key::S) || GetKey(leopph::Key::DownArrow)) {
							posDelta += leopph::Vector3::Backward();
						}

						Normalize(posDelta);

						if (GetKey(leopph::Key::Shift)) {
							posDelta *= 2;
						}

						editorCam.position += editorCam.orientation.Rotate(posDelta) * leopph::get_frame_time() * 2;

						auto const [mouseX, mouseY]{ leopph::gWindow.GetMouseDelta() };
						auto constexpr sens{ 0.05f };

						editorCam.orientation = leopph::Quaternion{ leopph::Vector3::Up(), static_cast<leopph::f32>(mouseX) * sens } * editorCam.orientation;
						editorCam.orientation *= leopph::Quaternion{ leopph::Vector3::Right(), static_cast<leopph::f32>(mouseY) * sens };
					}

					leopph::gRenderer.DrawSceneView(editorCam);
					ImGui::Image(leopph::gRenderer.GetSceneFrame(), contentRegionSize);

					auto const editorCamViewMat{ leopph::Matrix4::LookAtLH(editorCam.position, editorCam.position + editorCam.orientation.Rotate(leopph::Vector3::Forward()), leopph::Vector3::Up()) };
					auto const editorCamProjMat{ leopph::Matrix4::PerspectiveAsymZLH(editorCam.fovVertRad, ImGui::GetWindowWidth() / ImGui::GetWindowHeight(), editorCam.nearClip, editorCam.farClip) };

					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
					ImGuizmo::AllowAxisFlip(false);
					ImGuizmo::SetDrawlist();

					static bool showGrid{ true };

					if (GetKeyDown(leopph::Key::G)) {
						showGrid = !showGrid;
					}

					if (showGrid) {
						ImGuizmo::DrawGrid(editorCamViewMat.GetData(), editorCamProjMat.GetData(), leopph::Matrix4::Identity().GetData(), editorCam.farClip);
					}

					if (auto const selectedEntity{ dynamic_cast<leopph::Entity*>(gSelected) }; selectedEntity) {
						static auto op{ ImGuizmo::OPERATION::TRANSLATE };

						if (!io.WantTextInput && !isMovingSceneCamera) {
							if (GetKeyDown(leopph::Key::T)) {
								op = ImGuizmo::TRANSLATE;
							}
							if (GetKeyDown(leopph::Key::R)) {
								op = ImGuizmo::ROTATE;
							}
							if (GetKeyDown(leopph::Key::S)) {
								op = ImGuizmo::SCALE;
							}
							if (GetKeyDown(leopph::Key::F)) {
								editorCam.position = selectedEntity->GetTransform().GetWorldPosition() - leopph::Vector3::Forward() * 2;
								editorCam.orientation = selectedEntity->GetTransform().GetLocalRotation();
							}
						}

						if (leopph::Matrix4 modelMat{ selectedEntity->GetTransform().GetModelMatrix() }; Manipulate(editorCamViewMat.GetData(), editorCamProjMat.GetData(), op, ImGuizmo::MODE::LOCAL, modelMat.GetData())) {
							leopph::Vector3 pos, euler, scale;
							ImGuizmo::DecomposeMatrixToComponents(modelMat.GetData(), pos.GetData(), euler.GetData(), scale.GetData());
							selectedEntity->GetTransform().SetWorldPosition(pos);
							selectedEntity->GetTransform().SetWorldRotation(leopph::Quaternion::FromEulerAngles(euler));
							selectedEntity->GetTransform().SetWorldScale(scale);
						}
					}
				}
				else {
					ImGui::PopStyleVar();
				}
				ImGui::End();

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

								if (ImGui::MenuItem("Import Asset")) {
									if (nfdchar_t* selectedPath{ nullptr }; NFD_OpenDialog(nullptr, nullptr, &selectedPath) == NFD_OKAY) {
										auto const importAsset{
											[selectedPath, &resources, &projDirAbs, &assetDirRel, &createAssetMetaFile] {
												std::filesystem::path srcPath{ selectedPath };
												srcPath = absolute(srcPath);
												std::free(selectedPath);

												auto const dstPath{ IndexFileNameIfNeeded(projDirAbs / assetDirRel / srcPath.filename()) };
												copy(srcPath, dstPath);

												auto asset{ leopph::editor::LoadAsset(dstPath) };
												createAssetMetaFile(*asset);

												resources[std::move(dstPath)] = std::move(asset);
											}
										};

										std::thread loaderThread{ LoadAndBlockEditor, std::ref(io), importAsset };
										loaderThread.detach();
									}

									ImGui::CloseCurrentPopup();
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
