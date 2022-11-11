#include "Serialization.hpp"

#include <Components.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Time.hpp>
#include <Entity.hpp>
#include <OnGui.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
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
		for (auto const& entity : leopph::GetEntities(entities)) {
			leopph::DeleteEntity(entity);
		}
	}


	YAML::Node gSerializedSceneBackup;
}


int WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, [[maybe_unused]] _In_ wchar_t*, [[maybe_unused]] _In_ int) {
	if (!leopph::platform::init_platform_support()) {
		return 1;
	}

	leopph::platform::set_window_borderless(false);
	leopph::platform::set_window_windowed_client_area_size({ 1280, 720 });
	leopph::platform::SetIgnoreManagedRequests(true);

	if (!leopph::rendering::InitRenderer()) {
		return 2;
	}

	leopph::rendering::SetGameResolution({ 960, 540 });
	leopph::rendering::SetSyncInterval(1);

	if (!leopph::initialize_managed_runtime()) {
		return 3;
	}

	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	auto const iniFilePath{ std::filesystem::path{ leopph::platform::GetExecutablePath() }.remove_filename() /= "editorconfig.ini" };
	auto const iniFilePathStr{ leopph::platform::WideToUtf8(iniFilePath.c_str()) };
	io.IniFilename = iniFilePathStr.c_str();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	leopph::SetImGuiContext(ImGui::GetCurrentContext());

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(leopph::platform::get_hwnd());
	ImGui_ImplDX11_Init(leopph::rendering::GetDevice(), leopph::rendering::GetImmediateContext());

	leopph::platform::SetEventHook(EditorImGuiEventHook);

	bool runGame{ false };
	bool showDemoWindow{ false };

	leopph::init_time();

	while (!leopph::platform::should_window_close()) {
		if (!leopph::platform::process_platform_events()) {
			return 4;
		}

		if (runGame) {
			leopph::init_behaviors();
			leopph::tick_behaviors();
			leopph::tack_behaviors();

			if (leopph::platform::GetKeyDown(leopph::platform::Key::Escape)) {
				runGame = false;
				leopph::platform::SetEventHook(EditorImGuiEventHook);
				leopph::platform::confine_cursor(false);
				leopph::platform::hide_cursor(false);
				leopph::rendering::SetSyncInterval(1);
				CloseCurrentScene();
				leopph::editor::DeserializeScene(gSerializedSceneBackup);
			}
		}
		else {
			if (leopph::platform::GetKeyDown(leopph::platform::Key::F5)) {
				runGame = true;
				leopph::platform::SetEventHook({});
				leopph::rendering::SetSyncInterval(0);
				gSerializedSceneBackup = leopph::editor::SerializeScene();
			}
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport();

		if (showDemoWindow) {
			ImGui::ShowDemoWindow();
		}

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open")) {
					MessageBoxW(leopph::platform::get_hwnd(), L"Placeholder", L"Placeholder", 0);
				}

				if (ImGui::MenuItem("Save")) {
					if (!runGame) {
						std::ofstream out{ "scene.yaml" };
						YAML::Emitter emitter{ out };
						auto const serializedScene = leopph::editor::SerializeScene();
						emitter << serializedScene;
						gSerializedSceneBackup = serializedScene;
					}
				}

				if (ImGui::MenuItem("Load")) {
					CloseCurrentScene();
					auto const serializedScene = YAML::LoadFile("scene.yaml");
					leopph::editor::DeserializeScene(serializedScene);
					gSerializedSceneBackup = serializedScene;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Create")) {
				if (ImGui::MenuItem("Entity")) {
					leopph::Entity::Create()->CreateComponent<leopph::Transform>();
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

		static std::optional<std::size_t> selectedEntityIndex;
		static std::vector<leopph::Entity*> entities;
		leopph::GetEntities(entities);

		if (selectedEntityIndex && *selectedEntityIndex >= entities.size()) {
			selectedEntityIndex.reset();
		}

		if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
			for (std::size_t i = 0; i < entities.size(); i++) {
				ImGui::PushID(static_cast<int>(i));
				if (ImGui::Selectable(entities[i]->name.data(), selectedEntityIndex && *selectedEntityIndex == i)) {
					selectedEntityIndex = i;
				}

				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::Button("Delete")) {
						leopph::DeleteEntity(entities[i]);
					}
					ImGui::EndPopup();
				}

				ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);
				ImGui::PopID();
			}
		}
		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Entity Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
			if (selectedEntityIndex) {
				auto const& entity = entities[*selectedEntityIndex];

				static std::string entityName;
				entityName = entity->name;

				if (ImGui::BeginTable("Property Widgets", 2)) {
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::PushItemWidth(FLT_MIN);
					ImGui::Text("Name");

					ImGui::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					if (ImGui::InputText("##EntityName", &entityName)) {
						entity->name = entityName;
					}

					ImGui::EndTable();
				}

				static std::vector<leopph::Component*> components;

				for (auto const& component : entity->GetComponents(components)) {
					auto const obj = component->GetManagedObject();
					auto const klass = mono_object_get_class(obj);

					auto const componentNodeId = mono_class_get_name(klass);
					if (ImGui::TreeNodeEx(componentNodeId, ImGuiTreeNodeFlags_DefaultOpen)) {
						ImGui::Separator();
						component->OnGui();
						ImGui::TreePop();
					}

					if (ImGui::BeginPopupContextItem(componentNodeId)) {
						if (ImGui::Button("Delete")) {
							entities[*selectedEntityIndex]->DeleteComponent(component);
						}
						ImGui::EndPopup();
					}
					ImGui::OpenPopupOnItemClick(componentNodeId, ImGuiPopupFlags_MouseButtonRight);
				}

				auto constexpr addNewComponentLabel = "Add New Component";
				ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(addNewComponentLabel).x) * 0.5f);
				ImGui::Button(addNewComponentLabel);

				if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
					for (auto const& componentClass : leopph::GetComponentClasses()) {
						auto const componentName = mono_class_get_name(componentClass);
						ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(componentName).x) * 0.5f);
						if (ImGui::Button(componentName)) {
							entity->CreateComponent(componentClass);
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();

		ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

		ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
			ImGui::PopStyleVar();

			leopph::Extent2D<leopph::u32> const resolutions[]{ {960, 540}, {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160} };
			char const* const resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
			static int selectedRes = 0;

			if (ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7)) {
				if (selectedRes != 0) {
					leopph::rendering::SetGameResolution(resolutions[selectedRes - 1]);
				}
			}

			auto const gameRes = leopph::rendering::GetGameResolution();
			auto const contentRegionSize = ImGui::GetContentRegionAvail();
			leopph::Extent2D<leopph::u32> const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };
			ImVec2 frameDisplaySize;

			if (selectedRes == 0) {
				if (viewportRes.width != gameRes.width || viewportRes.height != gameRes.height) {
					leopph::rendering::SetGameResolution(viewportRes);
				}

				frameDisplaySize = contentRegionSize;
			}
			else {
				leopph::f32 const scale = std::min(contentRegionSize.x / static_cast<leopph::f32>(gameRes.width), contentRegionSize.y / static_cast<leopph::f32>(gameRes.height));
				frameDisplaySize = ImVec2(gameRes.width * scale, gameRes.height * scale);
			}

			if (!leopph::rendering::DrawGame()) {
				return 5;
			}
			ImGui::Image(reinterpret_cast<void*>(leopph::rendering::GetGameFrame()), frameDisplaySize);
		}
		else {
			ImGui::PopStyleVar();
		}
		ImGui::End();

		ImGui::Render();

		leopph::rendering::BindAndClearSwapChain();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		leopph::rendering::Present();

		leopph::measure_time();
	}

	CloseCurrentScene();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	leopph::cleanup_managed_runtime();
	leopph::rendering::CleanupRenderer();
	leopph::platform::cleanup_platform_support();
	return 0;
}