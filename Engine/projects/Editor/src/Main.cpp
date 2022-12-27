#include "Serialization.hpp"
#include "Widgets.hpp"
#include "Material.hpp"
#include "Time.hpp"

#include <Components.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Time.hpp>
#include <Entity.hpp>
#include <OnGui.hpp>
#include <Scene.hpp>
#include <SceneManager.hpp>
#include <Systems.hpp>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>

#include <mono/metadata/reflection.h>

#include <YamlInclude.hpp>
#include <nfd.h>

#include <algorithm>
#include <atomic>
#include <DirectXMath.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using leopph::Vector3;
using leopph::Quaternion;
using leopph::f32;

extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

namespace {
	auto EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) -> bool {
		return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	}


	auto CloseCurrentScene() -> void {
		static std::vector<leopph::Entity*> entities;
		for (auto const& entity : leopph::Entity::GetAllEntities(entities)) {
			entity->GetScene().DestroyEntity(entity);
		}
	}


	YAML::Node gSerializedSceneBackup;
	std::atomic<bool> gLoading{ false };

	auto LoadAndBlockEditor(ImGuiIO& io, std::function<void()> const& fun) -> void {
		auto const oldFlags{ io.ConfigFlags };
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
		gLoading = true;
		fun();
		io.ConfigFlags = oldFlags;
		gLoading = false;
	};


	/*auto ImportAsset(ImGuiIO& io) -> void {
		if (nfdchar_t* selectedPath{ nullptr }; NFD_OpenDialog(nullptr, nullptr, &selectedPath) == NFD_OKAY) {
			auto const LoadAndAddAssetToProject = [selectedPath] {
				if (auto const asset = leopph::editor::ImportAsset(selectedPath, gProject->folder); asset) {
					gProject->assets.emplace_back(asset);
				}
				std::free(selectedPath);
			};

			std::thread loaderThread{ LoadAndBlockEditor, std::ref(io), LoadAndAddAssetToProject };
			loaderThread.detach();
		}
	}*/


	leopph::Object* gSelected{ nullptr };
	std::optional<std::filesystem::path> gWorkingDir;
	std::optional<std::filesystem::path> gRootDir;
	std::vector<std::shared_ptr<leopph::Object>> gAssets;


	auto DiscoverAssetsInProjectFolder() -> void {
		for (auto const& childEntry : std::filesystem::recursive_directory_iterator{ *gRootDir }) {
			if (!childEntry.is_directory() && childEntry.path().extension() == ".leopphasset") {
				try {
					std::ifstream in{ childEntry.path(), std::ios::binary };
					std::vector<leopph::u8> bytes;
					bytes.insert(std::begin(bytes), std::istreambuf_iterator<char>{ in }, std::istreambuf_iterator<char>{});
					if (bytes.size() < sizeof(leopph::Object::Type)) {
						throw std::runtime_error{ std::format("Failed to deserialize asset at {}, because the file is corrupt.", childEntry.path().string()) };
					}
					auto const type{ *reinterpret_cast<leopph::Object::Type const*>(std::span{ bytes }.first<sizeof(leopph::Object::Type)>().data()) };
					std::shared_ptr<leopph::Object> asset;

					switch (type) {
						case leopph::Object::Type::Material: {
							asset = std::make_shared<leopph::Material>();
							break;
						}

						default:
							throw std::runtime_error{ std::format("Unknown asset type detected while parsing {}.", childEntry.path().string()) };
					}

					std::ignore = asset->DeserializeBinary(bytes);
					gAssets.emplace_back(std::move(asset));
				}
				catch (std::exception const& ex) {
					MessageBoxA(nullptr, ex.what(), "Error", MB_ICONERROR);
				}
			}
		}
	}
}


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, [[maybe_unused]] _In_ wchar_t*, [[maybe_unused]] _In_ int) -> int {
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

		leopph::init_time();

		while (!leopph::gWindow.IsQuitSignaled()) {
			leopph::gWindow.ProcessEvents();

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			if (!gWorkingDir || !gRootDir) {
				auto flags{ ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings };
				auto const viewport{ ImGui::GetMainViewport() };
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);

				if (ImGui::Begin("Open Project", nullptr, flags)) {
					auto const openProjectButtonLabel{ "Open Project" };
					auto const windowSize{ ImGui::GetWindowSize() };
					auto const textSize{ ImGui::CalcTextSize(openProjectButtonLabel) };

					ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
					ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);

					if (ImGui::Button("Open Project")) {
						if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
							gWorkingDir = std::filesystem::path{ selectedPath };
							gRootDir = gWorkingDir;
							DiscoverAssetsInProjectFolder();
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
						leopph::gWindow.SetCursorConfinement(false);
						leopph::gWindow.SetCursorHiding(false);
						leopph::gRenderer.SetSyncInterval(1);
						CloseCurrentScene();
						leopph::editor::DeserializeScene(gSerializedSceneBackup);
					}
				}
				else {
					if (leopph::GetKeyDown(leopph::Key::F5)) {
						runGame = true;
						leopph::gWindow.SetEventHook({});
						leopph::gRenderer.SetSyncInterval(0);
						gSerializedSceneBackup = leopph::editor::SerializeScene();
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
						/*if (ImGui::MenuItem("Open Project")) {
							if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
								auto const LoadAndAssignProject = [selectedPath] {
									gProject = leopph::editor::LoadProject(selectedPath);
									std::free(selectedPath);
								};

								std::thread loaderThread{ LoadAndBlockEditor, std::ref(io), LoadAndAssignProject };
								loaderThread.detach();
							}
						}

						if (ImGui::MenuItem("Import Asset")) {
							ImportAsset(io);
						}

						if (ImGui::MenuItem("Close Project")) {
							gProject = nullptr;
						}*/

						if (ImGui::MenuItem("Save Test Scene")) {
							if (!runGame) {
								std::ofstream out{ "scene.yaml" };
								YAML::Emitter emitter{ out };
								auto const serializedScene = leopph::editor::SerializeScene();
								emitter << serializedScene;
								gSerializedSceneBackup = serializedScene;
							}
						}

						if (ImGui::MenuItem("Load Test Scene")) {
							CloseCurrentScene();
							auto const serializedScene = YAML::LoadFile("scene.yaml");
							leopph::editor::DeserializeScene(serializedScene);
							gSerializedSceneBackup = serializedScene;
						}

						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Create")) {
						if (ImGui::MenuItem("Entity")) {
							auto const entity{ leopph::SceneManager::GetActiveScene()->CreateEntity() };
							entity->CreateManagedObject("leopph", "Entity");

							auto transform = std::make_unique<leopph::Transform>();
							transform->CreateManagedObject("leopph", "Transform");

							entity->AddComponent(std::move(transform));
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

				static std::vector<leopph::Entity*> entities;
				leopph::Entity::GetAllEntities(entities);


				if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
					for (std::size_t i = 0; i < entities.size(); i++) {
						ImGui::PushID(static_cast<int>(i));
						if (ImGui::Selectable(entities[i]->GetName().data(), gSelected == entities[i])) {
							gSelected = entities[i];
						}

						if (ImGui::BeginPopupContextItem()) {
							if (ImGui::MenuItem("Delete")) {
								entities[i]->GetScene().DestroyEntity(entities[i]);
								leopph::Entity::GetAllEntities(entities);
								gSelected = nullptr;
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}

						ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);
						ImGui::PopID();
					}
				}
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

				if (ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
					if (gSelected) {
						gSelected->OnGui();
					}
				}
				ImGui::End();

				ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

				ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

				if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
					ImGui::PopStyleVar();

					leopph::Extent2D<leopph::u32> constexpr resolutions[]{ { 960, 540 }, { 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 } };
					char const* const resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
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
						frameDisplaySize = ImVec2(gameRes.width * scale, gameRes.height * scale);
					}

					leopph::gRenderer.DrawGame();
					ImGui::Image(reinterpret_cast<void*>(leopph::gRenderer.GetGameFrame()), frameDisplaySize);
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
					auto const sceneRes{ leopph::gRenderer.GetSceneResolution() };
					auto const contentRegionSize{ ImGui::GetContentRegionAvail() };
					leopph::Extent2D const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };

					if (viewportRes.width != sceneRes.width || viewportRes.height != sceneRes.height) {
						leopph::gRenderer.SetSceneResolution(viewportRes);
					}

					static leopph::EditorCamera editorCam{
						.position = Vector3{ 0, 0, 0 },
						.forward = Vector3::forward(),
						.nearClip = 0.03f,
						.farClip = 300.f,
						.fovVertRad = leopph::to_radians(90),
					};

					if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
						leopph::gWindow.SetCursorConfinement(true);
						leopph::gWindow.SetCursorHiding(true);
						Vector3 posDelta{ 0, 0, 0 };
						if (GetKey(leopph::Key::W) || GetKey(leopph::Key::UpArrow)) {
							posDelta += Vector3::forward();
						}
						if (GetKey(leopph::Key::A) || GetKey(leopph::Key::LeftArrow)) {
							posDelta += Vector3::left();
						}
						if (GetKey(leopph::Key::D) || GetKey(leopph::Key::RightArrow)) {
							posDelta += Vector3::right();
						}
						if (GetKey(leopph::Key::S) || GetKey(leopph::Key::DownArrow)) {
							posDelta += Vector3::backward();
						}
						posDelta.normalize();

						if (GetKey(leopph::Key::Shift)) {
							posDelta *= 2;
						}
						editorCam.position += posDelta * leopph::get_frame_time() * 2;
						auto const mouseDelta{ leopph::gWindow.GetMouseDelta() };
						auto constexpr sens{ 0.05f };
						editorCam.forward = Quaternion{ Vector3::up(), static_cast<float>(mouseDelta.x) * sens }.Rotate(editorCam.forward);
						editorCam.forward = Quaternion{ cross(editorCam.forward, Vector3::up()), static_cast<float>(mouseDelta.y) * sens }.Rotate(editorCam.forward);
					}
					else {
						leopph::gWindow.SetCursorConfinement(false);
						leopph::gWindow.SetCursorHiding(false);
					}

					leopph::gRenderer.DrawSceneView(editorCam);
					ImGui::Image(leopph::gRenderer.GetSceneFrame(), contentRegionSize);

					if (auto const selectedEntity{ dynamic_cast<leopph::Entity*>(gSelected) }; selectedEntity) {
						static auto op{ ImGuizmo::OPERATION::TRANSLATE };
						if (GetKeyDown(leopph::Key::T)) {
							op = ImGuizmo::TRANSLATE;
						}
						if (GetKeyDown(leopph::Key::R)) {
							op = ImGuizmo::ROTATE;
						}
						if (GetKeyDown(leopph::Key::S)) {
							op = ImGuizmo::SCALE;
						}

						leopph::Matrix4 modelMat{ selectedEntity->GetTransform().GetModelMatrix() };
						auto const viewMat{
							[&] {
								DirectX::XMFLOAT3 const eye{ editorCam.position.get_data() };
								DirectX::XMFLOAT3 const forward{ editorCam.forward.get_data() };
								DirectX::XMFLOAT3 const up{ Vector3::up().get_data() };
								DirectX::XMFLOAT4X4 ret{};
								auto const lookTo{ DirectX::XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&forward), XMLoadFloat3(&up)) };
								auto const lookToTranspose{ XMMatrixTranspose(lookTo) };
								XMStoreFloat4x4(&ret, lookToTranspose);
								return ret;
							}()
						};
						auto const projMat{
							[] {
								DirectX::XMFLOAT4X4 ret{};
								XMStoreFloat4x4(&ret, XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(editorCam.fovVertRad, ImGui::GetWindowWidth() / ImGui::GetWindowHeight(), editorCam.nearClip, editorCam.farClip)));
								return ret;
							}()
						};

						ImGuizmo::AllowAxisFlip(false);
						ImGuizmo::SetDrawlist();
						ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
						if (Manipulate(viewMat.m[0], projMat.m[0], op, ImGuizmo::MODE::LOCAL, &modelMat[0][0])) {
							selectedEntity->GetTransform().SetWorldPosition(Vector3{ modelMat[3] });
						}
					}
				}
				else {
					ImGui::PopStyleVar();
				}
				ImGui::End();

				if (ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoCollapse)) {
					if (ImGui::BeginPopupContextItem("Assets")) {
						if (ImGui::MenuItem("Import Asset")) {
							ImGui::CloseCurrentPopup();
							//ImportAsset(io);
						}
						if (ImGui::BeginMenu("Create##CreateAsset")) {
							if (ImGui::MenuItem("Material##CreateMaterial")) {
								auto const mat{ new leopph::Material{} };
								mat->SetName("New Material");
								gAssets.emplace_back(mat);
								gSelected = mat;
								static std::vector<leopph::u8> bytes;
								mat->SerializeBinary(bytes);
								std::ofstream out{ (*gWorkingDir / mat->GetName()).replace_extension(".leopphasset"), std::ios::binary };
								std::ranges::copy(std::as_const(bytes), std::ostream_iterator<leopph::u8>{ out });
								bytes.clear();
							}
							ImGui::EndMenu();
						}
						ImGui::EndPopup();
					}

					ImGui::OpenPopupOnItemClick("Assets", ImGuiPopupFlags_MouseButtonRight);

					for (int i{ 0 }; auto const& asset : gAssets) {
						if (ImGui::Selectable(std::format("{}##{}{}", asset->GetName().data(), asset->GetName().data(), i).c_str(), gSelected == asset.get())) {
							gSelected = asset.get();
						}
						++i;
					}
				}
				ImGui::End();
			}

			ImGui::Render();

			leopph::gRenderer.BindAndClearSwapChain();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			leopph::gRenderer.Present();

			leopph::measure_time();
		}

		CloseCurrentScene();

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
