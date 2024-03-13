#include "Timing.hpp"
#include "Application.hpp"
#include "SettingsWindow.hpp"
#include "EntityHierarchyWindow.hpp"
#include "GameViewWindow.hpp"
#include "LoadingScreen.hpp"
#include "MainMenuBar.hpp"
#include "PropertiesWindow.hpp"
#include "PerformanceCounterWindow.hpp"
#include "ProjectWindow.hpp"
#include "SceneViewWindow.hpp"
#include "StartupScreen.hpp"
#include "MemoryAllocation.hpp"
#include "Platform.hpp"
#include "..\..\Sorcery\src\rendering\scene_renderer.hpp"
#include "Window.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <ImGuizmo.h>
#include <implot.h>

#include <shellapi.h>

#include <filesystem>
#include <string>
#include <cwchar>
#include <stdexcept>
#include <exception>


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE,
                     _In_ wchar_t* const lpCmdLine, [[maybe_unused]] _In_ int) -> int {
  try {
    sorcery::gWindow.StartUp();
    sorcery::gRenderer.StartUp();

    sorcery::timing::SetTargetFrameRate(sorcery::mage::SettingsWindow::DEFAULT_TARGET_FRAME_RATE);

    ImGui::CreateContext();
    auto& imGuiIo = ImGui::GetIO();
    auto const iniFilePath{std::filesystem::path{sorcery::GetExecutablePath()}.remove_filename() /= "editorconfig.ini"};
    auto const iniFilePathStr{sorcery::WideToUtf8(iniFilePath.c_str())};
    imGuiIo.IniFilename = iniFilePathStr.c_str();
    imGuiIo.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(sorcery::gWindow.GetNativeHandle())) {
      throw std::runtime_error{"Failed to initialize Dear ImGui Win32 Implementation."};
    }

    if (!ImGui_ImplDX11_Init(sorcery::gRenderer.GetDevice(), sorcery::gRenderer.GetThreadContext())) {
      throw std::runtime_error{"Failed to initialize Dear ImGui DX11 Implementation."};
    }

    extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    sorcery::gWindow.SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));

    bool runGame{false};

    sorcery::mage::Application app{imGuiIo};

    sorcery::timing::OnApplicationStart();

    if (std::wcscmp(lpCmdLine, L"") != 0) {
      int argc;
      auto const argv{CommandLineToArgvW(lpCmdLine, &argc)};

      if (argc > 0) {
        std::filesystem::path targetProjPath{argv[0]};
        targetProjPath = absolute(targetProjPath);
        app.OpenProject(targetProjPath);
      }

      if (argc > 1) {
        if (auto const scene{
          sorcery::gResourceManager.GetOrLoad<sorcery::Scene>(app.GetResourceDatabase().PathToGuid(argv[1]))
        }) {
          app.OpenScene(*scene);
        }
      }

      LocalFree(argv);
    }

    auto const projectWindow{std::make_unique<sorcery::mage::ProjectWindow>(app)};
    auto const sceneViewWindow{std::make_unique<sorcery::mage::SceneViewWindow>()};
    auto const gameViewWindow{std::make_unique<sorcery::mage::GameViewWindow>()};
    auto const propertiesWindow{std::make_unique<sorcery::mage::PropertiesWindow>(app)};
    auto const editorSettingsWindow{std::make_unique<sorcery::mage::SettingsWindow>(app, sceneViewWindow->GetCamera())};
    auto const mainMenuBar{std::make_unique<sorcery::mage::MainMenuBar>(app, *editorSettingsWindow)};
    auto const entityHierarchyWindow{std::make_unique<sorcery::mage::EntityHierarchyWindow>(app)};

    while (!sorcery::IsQuitSignaled()) {
      sorcery::ProcessEvents();
      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();
      ImGuizmo::BeginFrame();

      try {
        if (app.GetProjectDirectoryAbsolute().empty()) {
          DrawStartupScreen(app);
        } else {
          int static targetFrameRate{sorcery::timing::GetTargetFrameRate()};

          if (runGame) {
            if (GetKeyDown(sorcery::Key::Escape)) {
              runGame = false;
              sorcery::gWindow.SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));
              sorcery::gWindow.SetCursorLock(std::nullopt);
              sorcery::gWindow.SetCursorHiding(false);
              sorcery::timing::SetTargetFrameRate(targetFrameRate);
              app.GetScene().Load();
              app.SetSelectedObject(nullptr);
            }
          } else {
            if (GetKeyDown(sorcery::Key::F5)) {
              runGame = true;
              sorcery::gWindow.SetEventHandler(nullptr);
              sorcery::timing::SetTargetFrameRate(-1);
              app.GetScene().Save();
              targetFrameRate = sorcery::timing::GetTargetFrameRate();
            }
          }

          ImGui::DockSpaceOverViewport();

          if (app.IsEditorBusy()) {
            DrawLoadingScreen(app);
          }

          entityHierarchyWindow->Draw();
          gameViewWindow->Draw(runGame);
          sceneViewWindow->Draw(app);

          mainMenuBar->Draw();
          editorSettingsWindow->Draw();
          propertiesWindow->Draw();
          projectWindow->Draw();
          sorcery::mage::DrawPerformanceCounterWindow();
        }
      } catch (std::runtime_error const& err) {
        sorcery::DisplayError(err.what());
      }

      ImGui::Render();

      auto const ctx{sorcery::gRenderer.GetThreadContext()};
      sorcery::gRenderer.ClearAndBindMainRt(ctx);
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
      sorcery::gRenderer.BlitMainRtToSwapChain(ctx);

      Microsoft::WRL::ComPtr<ID3D11CommandList> cmdList;
      [[maybe_unused]] auto const hr{ctx->FinishCommandList(FALSE, cmdList.GetAddressOf())};
      assert(SUCCEEDED(hr));

      sorcery::gRenderer.ExecuteCommandList(cmdList.Get());
      sorcery::gRenderer.Present();

      sorcery::GetTmpMemRes().Clear();

      sorcery::timing::OnFrameEnd();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
  } catch (std::exception const& ex) {
    sorcery::DisplayError(ex.what());
  }

  sorcery::Object::DestroyAll();
  sorcery::gRenderer.ShutDown();
  sorcery::gWindow.ShutDown();
  return 0;
}
