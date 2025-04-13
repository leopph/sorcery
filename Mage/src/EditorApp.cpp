#include "EditorApp.hpp"

#include <ImGuizmo.h>
#include <imgui_impl_win32.h>
#include <implot.h>
#include <nfd.hpp>

#include "GUI.hpp"
#include "LoadingScreen.hpp"
#include "PerformanceCounterWindow.hpp"
#include "Platform.hpp"
#include "ResourceManager.hpp"
#include "StartupScreen.hpp"
#include "Timing.hpp"
#include "Window.hpp"

extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;


namespace sorcery::mage {
std::string_view const EditorApp::window_title_base_{"Mage"};


auto EditorApp::OnWindowFocusGain() -> void {
  if (!proj_dir_abs_.empty()) {
    resource_db_.Refresh();
  }
}


auto EditorApp::HandleBackgroundThreadException(std::exception const& ex) -> void {
  DisplayError(ex.what());
}


auto EditorApp::HandleUnknownBackgroundThreadException() -> void {
  DisplayError("Unknown error.");
}


EditorApp::EditorApp(std::span<std::string_view const> const args) :
  App{args},
  imgui_ctx_{ImGui::CreateContext()},
  imgui_io_{&ImGui::GetIO()},
  imgui_io_ini_path_{
    WideToUtf8((std::filesystem::path{GetExecutablePath()}.remove_filename() /= "editorconfig.ini").c_str())
  },
  window_focus_gain_listener_{
    GetWindow().OnWindowFocusGain.add_listener([this] { OnWindowFocusGain(); })
  } {
  imgui_io_->FontDefault = imgui_io_->Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\arial.ttf)", 14);
  imgui_io_->IniFilename = imgui_io_ini_path_.c_str();
  imgui_io_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  ImPlot::CreateContext();

  if (!ImGui_ImplWin32_Init(GetWindow().GetNativeHandle())) {
    throw std::runtime_error{"Failed to initialize Dear ImGui Win32 Implementation."};
  }

  imgui_renderer_.UpdateFonts();

  GetWindow().SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));

  GetWindow().SetTitle(std::string{window_title_base_});;
  SetImGuiContext(*ImGui::GetCurrentContext());
  SetGuiDarkMode(dark_mode_);

  timing::SetTargetFrameRate(SettingsWindow::DEFAULT_TARGET_FRAME_RATE);

  NFD::Init();

  auto project_loaded{false};

  for (auto const arg : args) {
    if (arg.starts_with("-project=")) {
      OpenProject(std::filesystem::absolute(arg.substr(9)));
      project_loaded = true;
      break;
    }
  }

  if (project_loaded) {
    for (auto const arg : args) {
      if (arg.starts_with("-scene=")) {
        OpenScene(GetResourceDatabase().PathToGuid(arg.substr(7)));
        break;
      }
    }
  }
}


EditorApp::~EditorApp() {
  WaitRenderJob();
  GetGraphicsDevice().WaitIdle();

  NFD::Quit();

  GetWindow().OnWindowFocusGain.remove_listener(window_focus_gain_listener_);

  ImGui_ImplWin32_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
}


auto EditorApp::BeginFrame() -> void {
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();
}


auto EditorApp::Update() -> void {
  if (GetProjectDirectoryAbsolute().empty()) {
    DrawStartupScreen(*this);
  } else {
    int static targetFrameRate{timing::GetTargetFrameRate()};

    if (game_is_running_) {
      for (std::vector<SceneObject*> scene_objects; auto const so : Object::FindObjectsOfType(scene_objects)) {
        if (so->IsUpdatable()) {
          so->Update();
        }
      }

      if (GetKeyDown(Key::Escape)) {
        game_is_running_ = false;
        GetWindow().SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));
        GetWindow().SetCursorLock(std::nullopt);
        GetWindow().SetCursorHiding(false);
        timing::SetTargetFrameRate(targetFrameRate);
        GetScene().Load();
        SetSelectedObject(nullptr);
      }
    } else {
      if (GetKeyDown(Key::F5)) {
        game_is_running_ = true;
        GetWindow().SetEventHandler(nullptr);
        GetScene().Save();
        targetFrameRate = timing::GetTargetFrameRate();
        timing::SetTargetFrameRate(-1);

        for (std::vector<SceneObject*> scene_objects; auto const so : Object::FindObjectsOfType(scene_objects)) {
          if (so->IsUpdatable()) {
            so->Start();
          }
        }
      }
    }

    ImGui::DockSpaceOverViewport();

    if (IsEditorBusy()) {
      DrawLoadingScreen(*this);
    }

    entity_hierarchy_window_.Draw();
    game_view_window_.Draw(game_is_running_);
    scene_view_window_.Draw(*this);

    main_menu_bar_.Draw();
    editor_settings_window_.Draw();
    properties_window_.Draw();
    project_window_.Draw();
    DrawPerformanceCounterWindow();
  }
}


auto EditorApp::EndFrame() -> void {
  ImGui::Render();
}


auto EditorApp::PrepareRender() -> void {
  imgui_renderer_.ExtractDrawData();
}


auto EditorApp::Render() -> void {
  imgui_renderer_.Render();
}


auto EditorApp::GetImGuiIo() const noexcept -> ImGuiIO const& {
  return *imgui_io_;
}


auto EditorApp::GetImGuiIo() noexcept -> ImGuiIO& {
  return *imgui_io_;
}


auto EditorApp::GetResourceDatabase() const noexcept -> ResourceDB const& {
  return resource_db_;
}


auto EditorApp::GetResourceDatabase() noexcept -> ResourceDB& {
  return resource_db_;
}


auto EditorApp::OpenScene(Guid const& guid) -> void {
  if (!guid.IsValid() || (scene_ && scene_->GetGuid() == guid)) {
    return;
  }

  if (auto const new_scene{GetResourceManager().GetOrLoad<Scene>(guid)}) {
    new_scene->Load();

    if (scene_) {
      GetResourceManager().Unload(scene_->GetGuid());
    }

    scene_.Reset(new_scene);
    scene_->SetActive();
    selected_object_ = nullptr;
  }
}


auto EditorApp::OpenNewScene() -> void {
  if (scene_) {
    GetResourceManager().Unload(scene_->GetGuid());
  }

  scene_ = GetResourceManager().Add(Create<Scene>());
  scene_->SetActive();
  selected_object_ = nullptr;
}


auto EditorApp::SaveCurrentSceneToFile() -> void {
  assert(scene_);
  scene_->Save();
  if (resource_db_.IsSavedResource(*scene_)) {
    resource_db_.SaveResource(*scene_);
  } else {
    constexpr nfdu8filteritem_t filter{ResourceManager::SCENE_RESOURCE_EXT.substr(1).data()};
    if (NFD::UniquePath dst;
      SaveDialog(dst, &filter, 1, resource_db_.GetResourceDirectoryAbsolutePath().string().c_str()) == NFD_OKAY) {
      if (auto const dstResDirRel{
        relative(std::filesystem::path{dst.get()}, resource_db_.GetResourceDirectoryAbsolutePath()) +=
        ResourceManager::SCENE_RESOURCE_EXT
      }; !dstResDirRel.empty()) {
        resource_db_.CreateResource(GetResourceManager().Remove<Scene>(scene_->GetGuid()), dstResDirRel);
      }
    }
  }
}


auto EditorApp::GetScene() const noexcept -> Scene& {
  assert(scene_);
  return *scene_;
}


auto EditorApp::GetSelectedObject() const noexcept -> Object* {
  return selected_object_;
}


auto EditorApp::SetSelectedObject(Object* const obj) noexcept -> void {
  selected_object_ = obj;
}


auto EditorApp::GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const& {
  return proj_dir_abs_;
}


auto EditorApp::OpenProject(std::filesystem::path const& targetPath) -> void {
  proj_dir_abs_ = absolute(targetPath);
  GetResourceManager().UnloadAll();
  resource_db_.ChangeProjectDir(proj_dir_abs_);
  GetWindow().SetTitle(std::string{window_title_base_} + " - " + targetPath.stem().string());

  OpenNewScene();
}


auto EditorApp::IsEditorBusy() const noexcept -> bool {
  return busy_;
}


auto EditorApp::IsGuiDarkMode() const noexcept -> bool {
  return dark_mode_;
}


auto EditorApp::SetGuiDarkMode(bool const darkMode) noexcept -> void {
  auto& style{ImGui::GetStyle()};

  if (darkMode) {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.68f, 0.68f, 0.69f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.77f, 0.77f, 0.78f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.22f, 0.22f, 0.22f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 0.97f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.53f, 0.46f, 0.62f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.46f);
  } else {
    ImGui::StyleColorsLight();
  }

  style.ChildBorderSize = 0;
  style.FrameBorderSize = 0;
  style.PopupBorderSize = 0;
  style.SeparatorTextBorderSize = 2;
  style.TabBorderSize = 0;
  style.WindowBorderSize = 0;
  style.ChildRounding = 4;
  style.FrameRounding = 4;
  style.GrabRounding = 4;
  style.PopupRounding = 4;
  style.ScrollbarRounding = 4;
  style.TabRounding = 4;
  style.WindowRounding = 4;

  GetWindow().UseImmersiveDarkMode(darkMode);
  dark_mode_ = darkMode;
}


auto EditorApp::OnEnterBusyExecution() -> BusyExecutionContext {
  auto isBusy{false};
  while (!busy_.compare_exchange_weak(isBusy, true)) {}

  BusyExecutionContext const ret{.imGuiConfigFlagsBackup = imgui_io_->ConfigFlags};

  imgui_io_->ConfigFlags |= ImGuiConfigFlags_NoMouse;
  imgui_io_->ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

  return ret;
}


auto EditorApp::OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void {
  imgui_io_->ConfigFlags = busyExecutionContext.imGuiConfigFlagsBackup;
  busy_ = false;
}
}
