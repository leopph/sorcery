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

#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>

#include <format>
#include <optional>
#include <fstream>
#include <limits>
#include <filesystem>
#include <algorithm>
#include <functional>
#include <string_view>
#include <cstring>
#include <string>
#include <vector>

using leopph::Vector3;
using leopph::Quaternion;
using leopph::f32;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
{
	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}


void DrawComponentMemberWidget(std::string_view const memberName, MonoType* const memberType, std::function<void* ()> const& getFunc, std::function<void(void**)> const& setFunc)
{
	std::string_view const memberTypeName = mono_type_get_name(memberType);

	/*if (mono_class_is_enum(mono_type_get_class(memberType)))
	{
		auto const underlyingType = mono_type_get_underlying_type(memberType);
		static std::vector<leopph::u64> currentValue;
		currentValue.resize(mono_type_stack_size())
		auto const values = leopph::GetEnumValues(memberType);
		auto const numValues = mono_array_length(values);



		for (std::size_t i = 0; i < numValues; i++)
		{

		}

	}*/

	if (memberTypeName == "leopph.Vector3")
	{
		float data[3];
		std::memcpy(data, getFunc(), sizeof(data));
		if (ImGui::DragFloat3(memberName.data(), data, 0.1f))
		{
			auto pData = &data[0];
			setFunc(reinterpret_cast<void**>(&pData));
		}
	}
	else if (memberTypeName == "leopph.Quaternion")
	{
		auto euler = reinterpret_cast<leopph::Quaternion*>(getFunc())->ToEulerAngles();
		if (ImGui::DragFloat3(memberName.data(), euler.get_data()))
		{
			auto quaternion = leopph::Quaternion::FromEulerAngles(euler[0], euler[1], euler[2]);
			auto pQuaternion = &quaternion;
			setFunc(reinterpret_cast<void**>(&pQuaternion));
		}
	}
	else if (memberTypeName == "System.Single")
	{
		float data;
		std::memcpy(&data, getFunc(), sizeof(data));
		if (ImGui::DragFloat(memberName.data(), &data))
		{
			auto pData = &data;
			setFunc(reinterpret_cast<void**>(&pData));
		}
	}
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

	leopph::rendering::SetGameResolution({ 960, 540 });

	if (!leopph::initialize_managed_runtime())
	{
		return 3;
	}

	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	char* exePath;
	_get_pgmptr(&exePath);
	std::filesystem::path iniFilePath{ exePath };
	iniFilePath.remove_filename() /= "editorconfig.ini";
	auto const iniFilePathStr = iniFilePath.string();
	io.IniFilename = iniFilePathStr.c_str();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(leopph::platform::get_hwnd());
	ImGui_ImplDX11_Init(leopph::rendering::GetDevice(), leopph::rendering::GetImmediateContext());

	leopph::platform::SetEventHook(EditorImGuiEventHook);

	bool runGame{ false };
	bool showDemoWindow{ false };

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

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport();

		if (showDemoWindow)
		{
			ImGui::ShowDemoWindow();
		}

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

			if (ImGui::BeginMenu("Debug"))
			{
				if (showDemoWindow)
				{
					if (ImGui::MenuItem("Hide Demo Window"))
					{
						showDemoWindow = false;
					}
				}
				else
				{
					if (ImGui::MenuItem("Show Demo Window"))
					{
						showDemoWindow = true;
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		static std::optional<std::size_t> selectedEntityIndex;

		if (selectedEntityIndex && *selectedEntityIndex >= leopph::gEntities.size())
		{
			selectedEntityIndex.reset();
		}

		if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse))
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

		ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Entity Properties", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			if (selectedEntityIndex)
			{
				auto const& entity = leopph::gEntities[*selectedEntityIndex];

				for (auto const& component : entity->components)
				{
					auto const obj = component->GetManagedObject();
					auto const klass = mono_object_get_class(obj);

					if (ImGui::TreeNodeEx(mono_class_get_name(klass), ImGuiTreeNodeFlags_DefaultOpen))
					{
						void* iter{ nullptr };
						while (auto const field = mono_class_get_fields(klass, &iter))
						{
							auto const refField = mono_field_get_object(leopph::GetManagedDomain(), klass, field);

							if (leopph::IsFieldExposed(refField))
							{
								DrawComponentMemberWidget(mono_field_get_name(field), mono_field_get_type(field), [field, obj]
								{
									return mono_object_unbox(mono_field_get_value_object(leopph::GetManagedDomain(), field, obj));
								}, [field, obj](void** data)
								{
									mono_field_set_value(obj, field, *data);
								});
							}
						}

						iter = nullptr;

						while (auto const prop = mono_class_get_properties(klass, &iter))
						{
							auto const refProp = mono_property_get_object(leopph::GetManagedDomain(), klass, prop);

							if (leopph::IsPropertyExposed(refProp))
							{
								DrawComponentMemberWidget(mono_property_get_name(prop), mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop))), [prop, obj]
								{
									return mono_object_unbox(mono_property_get_value(prop, obj, nullptr, nullptr));
								}, [prop, obj](void** data)
								{
									mono_property_set_value(prop, reinterpret_cast<void*>(obj), data, nullptr);
								});
							}
						}

						ImGui::TreePop();
					}
				}
				/*static std::string entityName;
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
				}*/
			}
		}
		ImGui::End();

		ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

		ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PopStyleVar();

			leopph::Extent2D<leopph::u32> const resolutions[]{ {960, 540}, {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160} };
			char const* const resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
			static int selectedRes = 0;

			if (ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7))
			{
				if (selectedRes != 0)
				{
					leopph::rendering::SetGameResolution(resolutions[selectedRes - 1]);
				}
			}

			auto const gameRes = leopph::rendering::GetGameResolution();
			auto const contentRegionSize = ImGui::GetContentRegionAvail();
			leopph::Extent2D<leopph::u32> const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };
			ImVec2 frameDisplaySize;

			if (selectedRes == 0)
			{
				if (viewportRes.width != gameRes.width || viewportRes.height != gameRes.height)
				{
					leopph::rendering::SetGameResolution(viewportRes);
				}

				frameDisplaySize = contentRegionSize;
			}
			else
			{
				leopph::f32 const scale = std::min(contentRegionSize.x / static_cast<leopph::f32>(gameRes.width), contentRegionSize.y / static_cast<leopph::f32>(gameRes.height));
				frameDisplaySize = ImVec2(gameRes.width * scale, gameRes.height * scale);
			}

			if (!leopph::rendering::DrawGame())
			{
				return 5;
			}
			ImGui::Image(reinterpret_cast<void*>(leopph::rendering::GetGameFrame()), frameDisplaySize);
		}
		ImGui::End();

		ImGui::Render();

		leopph::rendering::BindAndClearSwapChain();
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