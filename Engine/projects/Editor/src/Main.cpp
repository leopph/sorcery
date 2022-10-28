#include <Behavior.hpp>
#include <Managed.hpp>
#include <Platform.hpp>
#include <RenderCore.hpp>
#include <Time.hpp>
#include <Entity.hpp>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <format>
#include <optional>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


int main()
{
	if (!leopph::platform::init_platform_support())
	{
		return 1;
	}

	auto const renderer = leopph::RenderCore::create();

	if (!renderer)
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
	ImGui_ImplDX11_Init(renderer->get_device(), renderer->get_immediate_context());

	leopph::platform::SetEventHook([](HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
	{
		return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	});

	leopph::init_time();

	while (!leopph::platform::should_window_close())
	{
		if (!leopph::platform::process_platform_events())
		{
			return 4;
		}

		leopph::init_behaviors();
		leopph::tick_behaviors();
		leopph::tack_behaviors();

		if (!renderer->render())
		{
			return 5;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					MessageBoxW(leopph::platform::get_hwnd(), L"Placeholder", L"Placeholder", 0);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::SetNextWindowBgAlpha(1);
		ImGui::SetNextWindowPos(ImVec2{ 0, ImGui::GetItemRectSize().y });

		static std::optional<std::size_t> selectedEntityIndex;

		if (selectedEntityIndex.has_value() && *selectedEntityIndex >= leopph::gEntities.size())
		{
			selectedEntityIndex.reset();
		}

		ImGuiWindowFlags const entityListFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
		if (ImGui::Begin("Entities", nullptr, entityListFlags))
		{
			for (std::size_t i = 0; i < leopph::gEntities.size(); i++)
			{
				ImGui::PushID(static_cast<int>(i));
				if (ImGui::Selectable(leopph::gEntities[i]->get_name().data(), selectedEntityIndex.has_value() && *selectedEntityIndex == i))
				{
					selectedEntityIndex = i;
				}
				ImGui::PopID();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Entity Properties"))
		{

		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		renderer->present();

		leopph::measure_time();
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	leopph::cleanup_managed_runtime();
	leopph::platform::cleanup_platform_support();
}