#include "Serialization.hpp"
#include "AssetManagement.hpp"
#include "Widgets.hpp"

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

#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>

#include <YamlInclude.hpp>
#include <nfd.h>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using leopph::Vector3;
using leopph::Quaternion;
using leopph::f32;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	bool EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) {
		return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	}


	void CloseCurrentScene() {
		static std::vector<leopph::Entity*> entities;
		for (auto const& entity : leopph::Entity::GetAllEntities(entities)) {
			entity->GetScene().DestroyEntity(entity);
		}
	}


	YAML::Node gSerializedSceneBackup;
	std::unique_ptr<leopph::editor::Project> gProject;
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


	auto ImportAsset(ImGuiIO& io) -> void {
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
	}


	leopph::Object* gSelected{ nullptr };
}


int WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, [[maybe_unused]] _In_ wchar_t*, [[maybe_unused]] _In_ int) {
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

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

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
					}

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

				leopph::Extent2D<leopph::u32> constexpr resolutions[]{ {960, 540}, {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160} };
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

			if (ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoCollapse)) {
				if (gProject) {
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Import Asset")) {
							ImGui::CloseCurrentPopup();
							ImportAsset(io);
						}
						ImGui::EndPopup();
					}

					ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);

					for (auto const& asset : gProject->assets) {
						ImGui::Text(asset->GetPath().filename().string().c_str());
					}
				}
			}
			ImGui::End();

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