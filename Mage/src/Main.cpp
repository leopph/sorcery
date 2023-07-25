#include "Timing.hpp"
#include "EditorContext.hpp"

#include <Platform.hpp>
#include <Renderer.hpp>
#include <Systems.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

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
#include "PropertiesWindow.hpp"
#include "OpenScenePrompt.hpp"
#include "PerformanceCounterWindow.hpp"
#include "PhysicsManager.hpp"
#include "ProjectWindow.hpp"
#include "SceneViewWindow.hpp"
#include "StartupScreen.hpp"


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, _In_ wchar_t* const lpCmdLine, [[maybe_unused]] _In_ int) -> int {
  try {
    sorcery::gWindow.StartUp();
    sorcery::gRenderer.StartUp();
    sorcery::gPhysicsManager.StartUp();

    sorcery::gWindow.SetTitle("Mage");
    sorcery::gWindow.SetBorderless(false);
    sorcery::gWindow.SetWindowedClientAreaSize({ 1280, 720 });
    sorcery::gWindow.SetIgnoreManagedRequests(true);

    sorcery::gRenderer.SetSyncInterval(0);

    sorcery::timing::SetTargetFrameRate(sorcery::mage::DEFAULT_TARGET_FRAME_RATE);

    ImGui::CreateContext();
    auto& imGuiIo = ImGui::GetIO();
    auto const iniFilePath{ std::filesystem::path{ sorcery::GetExecutablePath() }.remove_filename() /= "editorconfig.ini" };
    auto const iniFilePathStr{ sorcery::WideToUtf8(iniFilePath.c_str()) };
    imGuiIo.IniFilename = iniFilePathStr.c_str();
    imGuiIo.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(sorcery::gWindow.GetHandle());
    ImGui_ImplDX11_Init(sorcery::gRenderer.GetDevice(), sorcery::gRenderer.GetImmediateContext());

    sorcery::gWindow.SetEventHook(sorcery::mage::EditorImGuiEventHook);

    bool runGame{ false };

    sorcery::mage::Context context{ imGuiIo };

    sorcery::timing::OnApplicationStart();

    if (std::wcscmp(lpCmdLine, L"") != 0) {
      int argc;
      auto const argv{ CommandLineToArgvW(lpCmdLine, &argc) };

      context.ExecuteInBusyEditor([argc, argv, &context] {
        if (argc > 0) {
          std::filesystem::path targetProjPath{ argv[0] };
          targetProjPath = absolute(targetProjPath);
          context.OpenProject(targetProjPath);
        }

        if (argc > 1) {
          std::filesystem::path targetScenePath{ argv[1] };
          targetScenePath = context.GetAssetDirectoryAbsolute() / targetScenePath;
          if (auto const targetScene{ context.GetResources().TryGetAssetAt(targetScenePath) }) {
            if (auto const targetSceneCast{ dynamic_cast<sorcery::Scene*>(targetScene) }; targetSceneCast) {
              context.OpenScene(*targetSceneCast);
            }
          }
        }

        LocalFree(argv);
      });
    }

    auto const projectWindow{ std::make_unique<sorcery::mage::ProjectWindow>(context) };
    auto const sceneViewWindow{ std::make_unique<sorcery::mage::SceneViewWindow>() };
    auto const gameViewWindow{ std::make_unique<sorcery::mage::GameViewWindow>() };
    auto const propertiesWindow{ std::make_unique<sorcery::mage::PropertiesWindow>(context) };

    while (!sorcery::gWindow.IsQuitSignaled()) {
      sorcery::gWindow.ProcessEvents();

      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();
      ImGuizmo::BeginFrame();

      if (context.GetProjectDirectoryAbsolute().empty()) {
        DrawStartupScreen(context);
      } else {
        int static targetFrameRate{ sorcery::timing::GetTargetFrameRate() };

        if (runGame) {
          if (GetKeyDown(sorcery::Key::Escape)) {
            runGame = false;
            sorcery::gWindow.SetEventHook(sorcery::mage::EditorImGuiEventHook);
            sorcery::gWindow.UnlockCursor();
            sorcery::gWindow.SetCursorHiding(false);
            sorcery::timing::SetTargetFrameRate(targetFrameRate);
            context.GetScene()->Load(context.GetFactoryManager());
            context.SetSelectedObject(nullptr);
          }

          sorcery::gPhysicsManager.Update();
        } else {
          if (GetKeyDown(sorcery::Key::F5)) {
            runGame = true;
            sorcery::gWindow.SetEventHook({});
            sorcery::timing::SetTargetFrameRate(-1);
            context.GetScene()->Save();
            targetFrameRate = sorcery::timing::GetTargetFrameRate();
          }
        }

        ImGui::DockSpaceOverViewport();

        if (context.IsEditorBusy()) {
          DrawLoadingScreen(context);
        }

        if (context.GetScene()) {
          DrawEntityHierarchyWindow(context);
          gameViewWindow->Draw(runGame);
          sceneViewWindow->Draw(context);
        } else {
          sorcery::mage::DrawOpenScenePrompt();
        }

        DrawMainMenuBar(context);
        propertiesWindow->Draw();
        projectWindow->Draw();
        sorcery::mage::DrawPerformanceCounterWindow();
      }

      ImGui::Render();

      sorcery::gRenderer.BindAndClearSwapChain();
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
      sorcery::gRenderer.Present();

      sorcery::timing::OnFrameEnd();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
  } catch (std::exception const& ex) {
    sorcery::DisplayError(ex.what());
  }

  sorcery::gPhysicsManager.ShutDown();
  sorcery::gRenderer.ShutDown();
  sorcery::gWindow.ShutDown();
  return 0;
}
