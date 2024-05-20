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
#include "rendering/imgui_renderer.hpp"
#include "job_system.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <ImGuizmo.h>
#include <implot.h>
#include <shellapi.h>

#include <atomic>
#include <barrier>
#include <cwchar>
#include <exception>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>


extern "C" {
__declspec(dllexport) extern UINT const D3D12SDKVersion{D3D12_SDK_VERSION};
__declspec(dllexport) extern char const* const D3D12SDKPath{".\\D3D12\\"};
}


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE,
                     _In_ wchar_t* const lpCmdLine, [[maybe_unused]] _In_ int) -> int {
  try {
#ifndef NDEBUG
    auto constexpr debug_graphics_device{true};
#else
    auto constexpr debug_graphics_device{false};
#endif

    auto const graphics_device{std::make_unique<sorcery::graphics::GraphicsDevice>(debug_graphics_device)};
    sorcery::g_engine_context.graphics_device.Reset(graphics_device.get());

    auto const window{std::make_unique<sorcery::Window>()};
    sorcery::g_engine_context.window.Reset(window.get());

    auto const swap_chain{
      graphics_device->CreateSwapChain(sorcery::graphics::SwapChainDesc{
        0, 0, 2, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_USAGE_RENDER_TARGET_OUTPUT, DXGI_SCALING_STRETCH
      }, static_cast<HWND>(window->GetNativeHandle()))
    };

    auto const render_manager{std::make_unique<sorcery::rendering::RenderManager>(*graphics_device)};
    sorcery::g_engine_context.render_manager.Reset(render_manager.get());

    auto const scene_renderer{
      std::make_unique<sorcery::rendering::SceneRenderer>(*window, *graphics_device, *render_manager)
    };
    sorcery::g_engine_context.scene_renderer.Reset(scene_renderer.get());

    sorcery::JobSystem job_system;
    sorcery::g_engine_context.job_system.Reset(&job_system);

    auto const resource_manager{std::make_unique<sorcery::ResourceManager>(job_system)};
    sorcery::g_engine_context.resource_manager.Reset(resource_manager.get());

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

    sorcery::mage::Application app{imGuiIo};

    auto const imgui_renderer{
      std::make_unique<sorcery::mage::ImGuiRenderer>(*graphics_device, *swap_chain, *render_manager)
    };

    bool game_is_running{false};
    bool window_resized{false};

    window->OnWindowSize.add_listener([&window_resized](sorcery::Extent2D<unsigned>) {
      window_resized = true;
    });

    auto const projectWindow{std::make_unique<sorcery::mage::ProjectWindow>(app)};
    auto const sceneViewWindow{std::make_unique<sorcery::mage::SceneViewWindow>()};
    auto const gameViewWindow{std::make_unique<sorcery::mage::GameViewWindow>()};
    auto const propertiesWindow{std::make_unique<sorcery::mage::PropertiesWindow>(app)};
    auto const editorSettingsWindow{std::make_unique<sorcery::mage::SettingsWindow>(app, sceneViewWindow->GetCamera())};
    auto const mainMenuBar{std::make_unique<sorcery::mage::MainMenuBar>(app, *editorSettingsWindow)};
    auto const entityHierarchyWindow{std::make_unique<sorcery::mage::EntityHierarchyWindow>(app)};

    std::barrier sync_point1{
      2, [&]() noexcept {
        if (window_resized) {
          graphics_device->WaitIdle();
          graphics_device->ResizeSwapChain(*swap_chain, 0, 0);
          window_resized = false;
        }
        sorcery::GetSingleFrameLinearMemory().Clear();
      }
    };
    std::barrier sync_point2{2, []() noexcept {}};
    std::atomic_flag quit_flag;

    auto const render_thread_func{
      [&] {
        while (!quit_flag.test()) {
          sync_point1.arrive_and_wait();
          scene_renderer->ExtractCurrentState();
          imgui_renderer->ExtractDrawData();

          sync_point2.arrive_and_wait();
          scene_renderer->Render();
          imgui_renderer->Render();
          swap_chain->Present();
          render_manager->EndFrame();
        }
      }
    };

    std::thread render_thread{render_thread_func};

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
          resource_manager->GetOrLoad<sorcery::Scene>(app.GetResourceDatabase().PathToGuid(argv[1]))
        }) {
          app.OpenScene(*scene);
        }
      }

      LocalFree(argv);
    }

    sorcery::timing::OnApplicationStart();

    while (!sorcery::IsQuitSignaled()) {
      sorcery::ProcessEvents();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();
      ImGuizmo::BeginFrame();

      try {
        if (app.GetProjectDirectoryAbsolute().empty()) {
          DrawStartupScreen(app);
        } else {
          int static targetFrameRate{sorcery::timing::GetTargetFrameRate()};

          if (game_is_running) {
            if (GetKeyDown(sorcery::Key::Escape)) {
              game_is_running = false;
              window->SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));
              window->SetCursorLock(std::nullopt);
              window->SetCursorHiding(false);
              sorcery::timing::SetTargetFrameRate(targetFrameRate);
              app.GetScene().Load();
              app.SetSelectedObject(nullptr);
            }
          } else {
            if (GetKeyDown(sorcery::Key::F5)) {
              game_is_running = true;
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
          gameViewWindow->Draw(game_is_running);
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

      sync_point1.arrive_and_wait();
      sync_point2.arrive_and_wait();

      sorcery::timing::OnFrameEnd();
    }

    quit_flag.test_and_set();
    render_thread.join();

    graphics_device->WaitIdle();

    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    sorcery::Object::DestroyAll();
  } catch (std::exception const& ex) {
    sorcery::DisplayError(ex.what());
  }

  return 0;
}
