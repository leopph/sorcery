#include "Widgets.hpp"
#include "Material.hpp"
#include "Time.hpp"
#include "Asset.hpp"
#include "ObjectFactoryManager.hpp"
#include "AssetStorage.hpp"
#include "EditorContext.hpp"

#include <TransformComponent.hpp>
#include <BehaviorComponent.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Entity.hpp>
#include <Scene.hpp>
#include <Systems.hpp>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <misc/cpp/imgui_stdlib.h>
#include <ImGuizmo.h>

#include <nfd.h>

#include <shellapi.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <format>
#include <fstream>
#include <ranges>
#include <string>
#include <cwchar>


extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;


namespace leopph::editor {
std::string_view constexpr RESOURCE_FILE_EXT{ ".leopphres" };

auto EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) -> bool {
	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
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

auto DrawStartupScreen(Context& context) -> void {
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
				context.OpenProject(selectedPath);
				std::free(selectedPath);
			}
		}
	}
	ImGui::End();
}

auto DrawMainMenuBar(Context& context, bool& showDemoWindow) -> void {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open Project")) {
				if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
					context.ExecuteInBusyEditor([selectedPath, &context] {
						context.OpenProject(selectedPath);
						std::free(selectedPath);
					});
				}
			}

			if (ImGui::MenuItem("Save Current Scene")) {
				if (context.GetScene()) {
					context.GetScene()->Save();
					context.SaveRegisteredNativeAsset(*context.GetScene());
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("Entity")) {
				auto& entity{ context.GetScene()->CreateEntity() };
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

auto DrawEntityHierarchyWindow(Context& context) -> void {
	if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
		auto constexpr baseFlags{ ImGuiTreeNodeFlags_OpenOnArrow };
		auto constexpr entityPayloadType{ "ENTITY" };

		if (ImGui::BeginDragDropTarget()) {
			if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
				static_cast<Entity*>(payload->Data)->GetTransform().SetParent(nullptr);
				ImGui::EndDragDropTarget();
			}
		}

		auto entities{ context.GetScene()->GetEntities() };

		for (std::size_t i = 0; i < entities.size(); i++) {
			std::function<void(Entity&)> displayEntityRecursive;
			displayEntityRecursive = [&displayEntityRecursive, &entities, &context](Entity& entity) -> void {
				ImGuiTreeNodeFlags nodeFlags{ baseFlags };

				if (entity.GetTransform().GetChildren().empty()) {
					nodeFlags |= ImGuiTreeNodeFlags_Leaf;
				}
				else {
					nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
				}

				if (context.GetSelectedObject() && context.GetSelectedObject()->GetGuid() == entity.GetGuid()) {
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				bool const nodeOpen{ ImGui::TreeNodeEx(entity.GetName().data(), nodeFlags) };

				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload(entityPayloadType, &entity, sizeof entity);
					ImGui::Text("%s", entity.GetName().data());
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
						static_cast<Entity*>(payload->Data)->GetTransform().SetParent(&entity.GetTransform());
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					context.SetSelectedObject(&entity);
				}

				bool deleted{ false };

				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete")) {
						entity.GetScene().DestroyEntity(entity);
						entities = context.GetScene()->GetEntities();
						context.SetSelectedObject(nullptr);
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

auto DrawObjectPropertiesWindow(Context& context) -> void {
	ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
		if (context.GetSelectedObject()) {
			context.GetFactoryManager().GetFor(context.GetSelectedObject()->GetSerializationType()).OnGui(context, *context.GetSelectedObject());
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

auto DrawSceneViewWindow(Context& context) -> void {
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

		if (auto const selectedEntity{ dynamic_cast<Entity*>(context.GetSelectedObject()) }; selectedEntity) {
			static auto op{ ImGuizmo::OPERATION::TRANSLATE };

			if (!context.GetImGuiIo().WantTextInput && !isMovingSceneCamera) {
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

auto DrawProjectWindow(Context& context) -> void {
	if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoCollapse)) {
		if (ImGui::BeginTable("ProjectWindowMainTable", 2, ImGuiTableFlags_Resizable)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::filesystem::path static selectedProjSubDir{ context.GetAssetDirectoryAbsolute() };
			[](this auto self, std::filesystem::path const& dir) -> void {
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
			}(context.GetAssetDirectoryAbsolute());

			ImGui::TableSetColumnIndex(1);

			auto constexpr contextMenuId{ "ProjectWindowContextMenu" };

			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				ImGui::OpenPopup(contextMenuId);
			}


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

					auto const importAsset{
						[&context, openFileDialog](Object::Type const targetAssetType) {
							auto& importer{ context.GetFactoryManager().GetFor(targetAssetType).GetImporter() };

							if (std::filesystem::path path; openFileDialog(importer.GetSupportedExtensions().c_str(), nullptr, path)) {
								context.ExecuteInBusyEditor([&context, &importer, path] {
									auto const srcPathAbs{ absolute(path) };
									auto const dstPath{ IndexFileNameIfNeeded(context.GetAssetDirectoryAbsolute() / srcPathAbs.filename()) };

									copy_file(srcPathAbs, dstPath);
									auto const guid{ Guid::Generate() };

									Importer::InputImportInfo const info{
										.src = dstPath,
										.guid = guid
									};

									auto const asset{ importer.Import(info, context.GetCacheDirectoryAbsolute()) };
									asset->SetName(dstPath.stem().string());
									asset->SetGuid(guid);

									context.GetResources().RegisterAsset(std::unique_ptr<Object>{ asset }, dstPath);
									context.CreateMetaFileForRegisteredAsset(*asset);
								});
							}
						}
					};

					if (ImGui::MenuItem("Mesh##ImportMeshAssetMenuItem")) {
						importAsset(Object::Type::Mesh);
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::MenuItem("Texture##ImportTextureAssetMenuItem")) {
						importAsset(Object::Type::Texture2D);
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndMenu();
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

auto DrawSceneOpenPrompt() {
	if (ImGui::Begin("No Open Scene##NoOpenScenePrompt", nullptr, ImGuiWindowFlags_None)) {
		auto const promptTextLabel{ "Create or open a scene from the Project Menu to start editing!" };
		auto const windowSize{ ImGui::GetWindowSize() };
		auto const textSize{ ImGui::CalcTextSize(promptTextLabel) };

		ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
		ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);

		ImGui::Text("%s", promptTextLabel);
	}
	ImGui::End();
}
}


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, _In_ wchar_t* const lpCmdLine, [[maybe_unused]] _In_ int) -> int {
	try {
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

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(leopph::gWindow.GetHandle());
		ImGui_ImplDX11_Init(leopph::gRenderer.GetDevice(), leopph::gRenderer.GetImmediateContext());

		leopph::gWindow.SetEventHook(leopph::editor::EditorImGuiEventHook);

		bool runGame{ false };
		bool showDemoWindow{ false };

		leopph::editor::Context context{ imGuiIo };

		leopph::init_time();

		if (std::wcscmp(lpCmdLine, L"") != 0) {
			int argc;
			auto const argv{ CommandLineToArgvW(lpCmdLine, &argc) };

			if (argc > 0) {
				std::filesystem::path targetProjPath{ argv[0] };
				targetProjPath = absolute(targetProjPath);
				context.OpenProject(targetProjPath);
			}

			if (argc > 1) {
				std::filesystem::path targetScenePath{ argv[1] };
				targetScenePath = context.GetAssetDirectoryAbsolute() / targetScenePath;
				if (auto const targetScene{ context.GetResources().TryGetAssetAt(targetScenePath) }) {
					context.OpenScene(dynamic_cast<leopph::Scene&>(*targetScene));
				}
			}

			LocalFree(argv);
		}

		while (!leopph::gWindow.IsQuitSignaled()) {
			leopph::gWindow.ProcessEvents();

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			if (context.GetProjectDirectoryAbsolute().empty()) {
				DrawStartupScreen(context);
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
						context.GetScene()->Load(context.GetFactoryManager());
						context.SetSelectedObject(nullptr);
					}
				}
				else {
					if (GetKeyDown(leopph::Key::F5)) {
						runGame = true;
						leopph::gWindow.SetEventHook({});
						leopph::gRenderer.SetSyncInterval(0);
						context.GetScene()->Save();
					}
				}

				ImGui::DockSpaceOverViewport();

				if (context.IsEditorBusy()) {
					ImGui::SetNextWindowPos(ImVec2(imGuiIo.DisplaySize.x * 0.5f, imGuiIo.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
					if (ImGui::Begin("LoadingIndicator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
						leopph::editor::DrawSpinner("##spinner", 15, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
					}
					ImGui::End();
				}

				if (showDemoWindow) {
					ImGui::ShowDemoWindow();
				}

				if (context.GetScene()) {
					leopph::editor::DrawEntityHierarchyWindow(context);
					leopph::editor::DrawGameViewWindow(runGame);
					leopph::editor::DrawSceneViewWindow(context);
				}
				else {
					leopph::editor::DrawSceneOpenPrompt();
				}

				DrawMainMenuBar(context, showDemoWindow);
				DrawObjectPropertiesWindow(context);
				DrawProjectWindow(context);
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
