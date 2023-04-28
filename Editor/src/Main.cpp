#include "Timing.hpp"
#include "EditorContext.hpp"

#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Systems.hpp>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <ImGuizmo.h>
#include <implot.h>

#include <shellapi.h>

#include <filesystem>
#include <ranges>
#include <string>
#include <cwchar>

#include "EditorSettingsWindow.hpp"
#include "EntityHierarchyWindow.hpp"
#include "GameViewWindow.hpp"
#include "ImGuiIntegration.hpp"
#include "LoadingScreen.hpp"
#include "MainMenuBar.hpp"
#include "ObjectPropertiesWindow.hpp"
#include "OpenScenePrompt.hpp"
#include "PerformanceCounterWindow.hpp"
#include "ProjectWindow.hpp"
#include "SceneViewWindow.hpp"
#include "StartupScreen.hpp"


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, _In_ wchar_t* const lpCmdLine, [[maybe_unused]] _In_ int) -> int {
	try {
		leopph::gWindow.StartUp();
		leopph::renderer::StartUp();
		leopph::gManagedRuntime.StartUp();

		leopph::gWindow.SetBorderless(false);
		leopph::gWindow.SetWindowedClientAreaSize({ 1280, 720 });
		leopph::gWindow.SetIgnoreManagedRequests(true);

		leopph::renderer::SetGameResolution({ 960, 540 });
		leopph::renderer::SetSyncInterval(0);

		leopph::timing::SetTargetFrameRate(leopph::editor::DEFAULT_TARGET_FRAME_RATE);

		ImGui::CreateContext();
		auto& imGuiIo = ImGui::GetIO();
		auto const iniFilePath{ std::filesystem::path{ leopph::GetExecutablePath() }.remove_filename() /= "editorconfig.ini" };
		auto const iniFilePathStr{ leopph::WideToUtf8(iniFilePath.c_str()) };
		imGuiIo.IniFilename = iniFilePathStr.c_str();
		imGuiIo.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImPlot::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(leopph::gWindow.GetHandle());
		ImGui_ImplDX11_Init(leopph::renderer::GetDevice(), leopph::renderer::GetImmediateContext());

		leopph::gWindow.SetEventHook(leopph::editor::EditorImGuiEventHook);

		bool runGame{ false };

		leopph::editor::Context context{ imGuiIo };

		leopph::timing::OnApplicationStart();

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
				int static targetFrameRate{ leopph::timing::GetTargetFrameRate() };

				if (runGame) {
					leopph::init_behaviors();
					leopph::tick_behaviors();
					leopph::tack_behaviors();

					if (GetKeyDown(leopph::Key::Escape)) {
						runGame = false;
						leopph::gWindow.SetEventHook(leopph::editor::EditorImGuiEventHook);
						leopph::gWindow.UnlockCursor();
						leopph::gWindow.SetCursorHiding(false);
						leopph::timing::SetTargetFrameRate(targetFrameRate);
						context.GetScene()->Load(context.GetFactoryManager());
						context.SetSelectedObject(nullptr);
					}
				}
				else {
					if (GetKeyDown(leopph::Key::F5)) {
						runGame = true;
						leopph::gWindow.SetEventHook({});
						leopph::timing::SetTargetFrameRate(-1);
						context.GetScene()->Save();
						targetFrameRate = leopph::timing::GetTargetFrameRate();
					}
				}

				ImGui::DockSpaceOverViewport();

				if (context.IsEditorBusy()) {
					DrawLoadingScreen(context);
				}

				if (context.GetScene()) {
					DrawEntityHierarchyWindow(context);
					leopph::editor::DrawGameViewWindow(runGame);
					DrawSceneViewWindow(context);
				}
				else {
					leopph::editor::DrawOpenScenePrompt();
				}

				DrawMainMenuBar(context);
				DrawObjectPropertiesWindow(context);
				DrawProjectWindow(context);
				leopph::editor::DrawPerformanceCounterWindow();
			}

			ImGui::Render();

			leopph::renderer::BindAndClearSwapChain();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			leopph::renderer::Present();

			leopph::timing::OnFrameEnd();
		}

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();
	}
	catch (std::exception const& ex) {
		leopph::DisplayError(ex.what());
	}

	leopph::gManagedRuntime.ShutDown();
	leopph::renderer::ShutDown();
	leopph::gWindow.ShutDown();
	return 0;
}
