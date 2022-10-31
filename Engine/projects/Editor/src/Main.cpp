#include "Serialization.hpp"

#include <Components.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Time.hpp>
#include <Entity.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <format>
#include <optional>
#include <fstream>
#include <limits>

using leopph::Vector3;
using leopph::Quaternion;
using leopph::f32;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
{
	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}


int main()
{
	if (!leopph::platform::init_platform_support())
	{
		return 1;
	}

	leopph::platform::set_window_borderless(false);
	leopph::platform::set_window_windowed_client_area_size({ 1280, 720 });
	leopph::platform::SetIgnoreManagedRequests(true);

	if (!leopph::rendering::InitRenderer())
	{
		return 2;
	}

	if (!leopph::initialize_managed_runtime())
	{
		return 3;
	}

	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.IniFilename = "";
	static_cast<void>(io);

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(leopph::platform::get_hwnd());
	ImGui_ImplDX11_Init(leopph::rendering::GetDevice(), leopph::rendering::GetImmediateContext());

	leopph::platform::SetEventHook(EditorImGuiEventHook);

	bool runGame{ false };

	leopph::init_time();

	while (!leopph::platform::should_window_close())
	{
		if (!leopph::platform::process_platform_events())
		{
			return 4;
		}

		if (runGame)
		{
			leopph::init_behaviors();
			leopph::tick_behaviors();
			leopph::tack_behaviors();

			if (leopph::platform::GetKeyDown(leopph::platform::Key::Escape))
			{
				runGame = false;
				leopph::platform::SetEventHook(EditorImGuiEventHook);
				leopph::platform::confine_cursor(false);
				leopph::platform::hide_cursor(false);
			}
		}
		else
		{
			if (leopph::platform::GetKeyDown(leopph::platform::Key::F5))
			{
				runGame = true;
				leopph::platform::SetEventHook({});
			}
		}

		if (!leopph::rendering::Render())
		{
			return 5;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					MessageBoxW(leopph::platform::get_hwnd(), L"Placeholder", L"Placeholder", 0);
				}

				if (ImGui::MenuItem("Save"))
				{
					std::ofstream out{ "scene.yaml" };
					Serialize(leopph::gEntities, out);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		auto const menuBarHeight = ImGui::GetItemRectSize().y;

		ImGui::SetNextWindowBgAlpha(1);
		ImGui::SetNextWindowPos(ImVec2{ 0,  menuBarHeight });

		static std::optional<std::size_t> selectedEntityIndex;

		if (selectedEntityIndex && *selectedEntityIndex >= leopph::gEntities.size())
		{
			selectedEntityIndex.reset();
		}

		ImGuiWindowFlags const entityListFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
		if (ImGui::Begin("Entities", nullptr, entityListFlags))
		{
			for (std::size_t i = 0; i < leopph::gEntities.size(); i++)
			{
				ImGui::PushID(static_cast<int>(i));
				if (ImGui::Selectable(leopph::gEntities[i]->name.data(), selectedEntityIndex && *selectedEntityIndex == i))
				{
					selectedEntityIndex = i;
				}
				ImGui::PopID();
			}
		}
		ImGui::End();

		ImGui::SetNextWindowPos({ static_cast<f32>(leopph::platform::get_window_current_client_area_size().width), menuBarHeight }, 0, ImVec2{ 1, 0 });

		if (ImGui::Begin("Entity Properties", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (selectedEntityIndex)
			{
				auto const& entity = leopph::gEntities[*selectedEntityIndex];

				static std::string entityName;
				entityName = entity->name;
				if (ImGui::InputText("Name", &entityName))
				{
					entity->name = entityName;
				}

				if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					Vector3 pos{ entity->transform->GetLocalPosition() };
					if (ImGui::DragFloat3("Local Positon", &pos[0], 0.1f))
					{
						entity->transform->SetLocalPosition(pos);
					}

					Vector3 rotEuler{ entity->transform->GetLocalRotation().ToEulerAngles() };
					if (ImGui::DragFloat3("Local Rotation", &rotEuler[0]))
					{
						entity->transform->SetLocalRotation(Quaternion::FromEulerAngles(rotEuler[0], rotEuler[1], rotEuler[2]));
					}

					Vector3 scale{ entity->transform->GetLocalScale() };
					if (ImGui::DragFloat3("Local Scale", &scale[0], 0.05f))
					{
						entity->transform->SetLocalScale(scale);
					}

					ImGui::TreePop();
				}

				static std::vector<leopph::Camera*> cameraComponents;
				cameraComponents.clear();
				entity->GetComponents<leopph::Camera>(cameraComponents);

				for ([[maybe_unused]] auto* const camera : cameraComponents)
				{
					if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
					{
						char const* const camTypeNames[]{ "Orthographic", "Perspective" };
						int currentType = camera->GetType() == leopph::Camera::Type::Orthographic ? 0 : 1;
						if (ImGui::Combo("Type", &currentType, camTypeNames, 2))
						{
							camera->SetType(currentType == 0 ? leopph::Camera::Type::Orthographic : leopph::Camera::Type::Perspective);
						}

						if (camera->GetType() == leopph::Camera::Type::Perspective)
						{
							f32 horizFovDeg{ camera->GetPerspectiveFov() };
							if (ImGui::DragFloat("Horizontal FOV", &horizFovDeg, 1.f, 1.f, 180.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
							{
								camera->SetPerspectiveFov(horizFovDeg);
							}
						}
						else
						{
							f32 horizSize{ camera->GetOrthographicSize() };
							if (ImGui::DragFloat("Horizontal Size", &horizSize, 1.f, 0.1f, std::numeric_limits<f32>::max(), "%.2f", ImGuiSliderFlags_AlwaysClamp))
							{
								camera->SetOrthoGraphicSize(horizSize);
							}
						}

						f32 nearClip{ camera->GetNearClipPlane() };
						if (ImGui::DragFloat("Near Clip Plane", &nearClip, 0.2f, 0.01f, std::numeric_limits<f32>::max(), "%.2f", ImGuiSliderFlags_AlwaysClamp))
						{
							camera->SetNearClipPlane(nearClip);
						}

						f32 farClip{ camera->GetFarClipPlane() };
						if (ImGui::DragFloat("Far Clip Plane", &farClip, 0.2f, nearClip + 0.1f, std::numeric_limits<f32>::max(), "%.2f", ImGuiSliderFlags_AlwaysClamp))
						{
							camera->SetFarClipPlane(farClip);
						}

						ImGui::TreePop();
					}
				}

				static std::vector<leopph::CubeModel*> cubeModelComponents;
				cubeModelComponents.clear();
				entity->GetComponents<leopph::CubeModel>(cubeModelComponents);

				for ([[maybe_unused]] auto* const cubeModel : cubeModelComponents)
				{
					if (ImGui::TreeNodeEx("Cube Model", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::TreePop();
					}
				}
			}
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		leopph::rendering::Present();

		leopph::measure_time();
	}

	leopph::gEntities.clear();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	leopph::cleanup_managed_runtime();
	leopph::rendering::CleanupRenderer();
	leopph::platform::cleanup_platform_support();
}