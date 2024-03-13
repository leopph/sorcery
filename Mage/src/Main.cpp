#include "Timing.hpp"
#include "Application.hpp"
#include "engine_context.hpp"
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
#include "Window.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>

#include <ImGuizmo.h>
#include <implot.h>

#include <shellapi.h>

#include <filesystem>
#include <string>
#include <cwchar>
#include <stdexcept>
#include <exception>


extern "C" {
__declspec(dllexport) UINT const D3D12SDKVersion{sorcery::graphics::kD3D12SdkVersion};
__declspec(dllexport) char const* const D3D12SDKPath{sorcery::graphics::kD3D12SdkPath};
}


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE,
                     _In_ wchar_t* const lpCmdLine, [[maybe_unused]] _In_ int) -> int {
  try {
    auto const window{std::make_unique<sorcery::Window>()};
    sorcery::g_engine_context.window.Reset(window.get());

#ifndef NDEBUG
    auto constexpr debug_graphics_device{true};
#else
    auto constexpr debug_graphics_device{false};
#endif

    auto const graphics_device{sorcery::graphics::GraphicsDevice::New(debug_graphics_device)};
    sorcery::g_engine_context.graphics_device.Reset(graphics_device.get());

    auto const render_manager{std::make_unique<sorcery::rendering::RenderManager>(*graphics_device)};
    sorcery::g_engine_context.render_manager.Reset(render_manager.get());

    auto const scene_renderer{std::make_unique<sorcery::rendering::SceneRenderer>(*render_manager, *window)};
    sorcery::g_engine_context.scene_renderer.Reset(scene_renderer.get());

    sorcery::timing::SetTargetFrameRate(sorcery::mage::SettingsWindow::DEFAULT_TARGET_FRAME_RATE);

    ImGui::CreateContext();
    auto& imGuiIo = ImGui::GetIO();
    auto const iniFilePath{std::filesystem::path{sorcery::GetExecutablePath()}.remove_filename() /= "editorconfig.ini"};
    auto const iniFilePathStr{sorcery::WideToUtf8(iniFilePath.c_str())};
    imGuiIo.IniFilename = iniFilePathStr.c_str();
    imGuiIo.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(window->GetNativeHandle())) {
      throw std::runtime_error{"Failed to initialize Dear ImGui Win32 Implementation."};
    }

    extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    window->SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));

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
      render_manager->BeginNewFrame();
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
              window->SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));
              window->SetCursorLock(std::nullopt);
              window->SetCursorHiding(false);
              sorcery::timing::SetTargetFrameRate(targetFrameRate);
              app.GetScene().Load();
              app.SetSelectedObject(nullptr);
            }
          } else {
            if (GetKeyDown(sorcery::Key::F5)) {
              runGame = true;
              window->SetEventHandler(nullptr);
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

      scene_renderer->Render();
      // TODO draw UI
      // TODO present

      sorcery::GetTmpMemRes().Clear();

      sorcery::timing::OnFrameEnd();
    }

    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    sorcery::Object::DestroyAll();
  } catch (std::exception const& ex) {
    sorcery::DisplayError(ex.what());
  }

  return 0;
}
