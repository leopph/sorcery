#include "editor_app.hpp"

#include <algorithm>
#include <ranges>
#include <vector>

#include <ImGuizmo.h>
#include <imgui_impl_win32.h>
#include <implot.h>
#include <nfd.hpp>

#include "gui_helpers.hpp"
#include "LoadingScreen.hpp"
#include "StartupScreen.hpp"
#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/timing.hpp"
#include "sorcery/host/command_line_args.hpp"
#include "sorcery/host/game_module.hpp"
#include "sorcery/host/platform.hpp"
#include "sorcery/host/window.hpp"
#include "sorcery/host/assets/asset_manager.hpp"
#include "sorcery/scene/Entity.hpp"
#include "sorcery/scene/environment_component.hpp"
#include "windows/PerformanceCounterWindow.hpp"


extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;


namespace sorcery::mage {
EditorApp::EditorApp(CommandLineArgs const& args) :
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

  GetWindow().SetTitle(std::string{window_title_base_});
  SetGuiDarkMode(dark_mode_);

  timing::SetTargetFrameRate(SettingsWindow::DEFAULT_TARGET_FRAME_RATE);

  NFD::Init();

  if (auto const game_mod_arg = args.GetArg("game-module")) {
    GetGameModule().Load(std::filesystem::absolute(ToUtf8Sv(*game_mod_arg)));
  }

  auto project_loaded{false};

  if (auto const proj_arg = args.GetArg("project")) {
    OpenProject(std::filesystem::absolute(ToUtf8Sv(*proj_arg)));
    project_loaded = true;
  }

  if (auto const scene_arg{args.GetArg("scene")}; project_loaded && scene_arg) {
    OpenScene(AssetId{GetAssetDatabase().PathToGuid(ToUtf8Sv(*scene_arg)), 0});
  }
}


EditorApp::~EditorApp() {
  WaitRenderJobNoThrow();
  GetGraphicsDevice().WaitIdle();

  NFD::Quit();

  GetWindow().OnWindowFocusGain.remove_listener(window_focus_gain_listener_);

  ImGui_ImplWin32_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
}


auto EditorApp::GetImGuiIo() const noexcept -> ImGuiIO const& {
  return *imgui_io_;
}


auto EditorApp::GetImGuiIo() noexcept -> ImGuiIO& {
  return *imgui_io_;
}


auto EditorApp::GetAssetDatabase() const noexcept -> AssetDB const& {
  return asset_db_;
}


auto EditorApp::GetAssetDatabase() noexcept -> AssetDB& {
  return asset_db_;
}


auto EditorApp::OpenScene(AssetId const& asset_id) -> void {
  if (!asset_id.IsValid() || (scene_ && scene_->GetAssetId() == asset_id)) {
    return;
  }

  if (auto const new_scene{GetAssetManager().Resolve<Scene>(asset_id)}) {
    LoadSceneIntoEditWorld(*new_scene, true);

    if (scene_) {
      GetAssetManager().Unload(scene_->GetAssetId());
    }

    scene_ = MakeAssetRef(new_scene.Get());
    selected_object_ = nullptr;
  }
}


auto EditorApp::OpenNewScene() -> void {
  if (scene_) {
    GetAssetManager().Unload(scene_->GetAssetId());
  }

  scene_ = GetAssetManager().Add(std::make_unique<Scene>());
  selected_object_ = nullptr;
  edit_world_.Clear();

  auto env_entity = std::make_unique<Entity>();
  env_entity->SetName("Environment");
  env_entity->AddComponent(std::make_unique<EnvironmentComponent>());
  edit_world_.AddEntity(std::move(env_entity));
}


auto EditorApp::SaveCurrentSceneToFile() -> void {
  assert(scene_);
  SaveWorldIntoScene(edit_world_, *scene_);
  if (asset_db_.IsSavedAsset(*scene_)) {
    asset_db_.SaveAssetToFile(*scene_);
  } else {
    constexpr nfdu8filteritem_t filter{"Scene files", AssetManager::SCENE_ASSET_EXT.substr(1).data()};
    if (NFD::UniquePath dst;
      SaveDialog(dst, &filter, 1,
        ToUntypedSv(asset_db_.GetAssetDirectoryAbsolutePath().u8string()).data()) == NFD_OKAY) {
      auto dst_asset_dir_rel{relative(std::filesystem::path{dst.get()}, asset_db_.GetAssetDirectoryAbsolutePath())};

      if (dst_asset_dir_rel.extension() != AssetManager::SCENE_ASSET_EXT) {
        dst_asset_dir_rel += AssetManager::SCENE_ASSET_EXT;
      }

      if (!dst_asset_dir_rel.empty()) {
        asset_db_.SaveAssetToFile(GetAssetManager().Remove<Scene>(scene_->GetAssetId()), dst_asset_dir_rel);
      }
    }
  }
}


auto EditorApp::GetScene() const noexcept -> Scene& {
  assert(scene_);
  return *scene_;
}


auto EditorApp::GetSelectedObject() const noexcept -> ObjectPtr<Object> {
  return selected_object_;
}


auto EditorApp::SetSelectedObject(ObjectPtr<Object> const obj) noexcept -> void {
  selected_object_ = obj;
}


auto EditorApp::GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const& {
  return proj_dir_abs_;
}


auto EditorApp::OpenProject(std::filesystem::path const& targetPath) -> void {
  proj_dir_abs_ = absolute(targetPath);
  GetAssetManager().UnloadAll();
  asset_db_.ChangeProjectDir(proj_dir_abs_);
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
    colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.24f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.30f, 0.34f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.38f, 1.00f, 1.00f);
    colors[ImGuiCol_CheckboxSelectedBg] = ImVec4(0.22f, 0.24f, 0.27f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.38f, 0.43f, 0.49f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.38f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.56f, 0.38f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.65f, 0.52f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.47f, 0.26f, 1.00f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.47f, 0.26f, 1.00f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.65f, 0.52f, 1.00f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.26f, 1.00f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_InputTextCursor] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.65f, 0.52f, 1.00f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.47f, 0.26f, 1.00f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_TreeLines] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_DragDropTargetBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_UnsavedMarker] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.35f);
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


auto EditorApp::GetRenderedWorld() -> World& {
  return edit_world_;
  //return game_is_running_ ? play_world_ : edit_world_;
}


auto EditorApp::BeginFrame() -> void {
  App::BeginFrame();
  tex_refs_.Reset();
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
      edit_world_.Update();

      if (GetKeyDown(Key::Escape)) {
        game_is_running_ = false;
        GetWindow().SetEventHandler(static_cast<void const*>(&ImGui_ImplWin32_WndProcHandler));
        GetWindow().SetCursorLock(std::nullopt);
        GetWindow().SetCursorHiding(false);
        timing::SetTargetFrameRate(targetFrameRate);
        LoadSceneIntoEditWorld(GetScene(), true);
        SetSelectedObject(nullptr);
      }
    } else {
      if (GetKeyDown(Key::F5)) {
        game_is_running_ = true;
        GetWindow().SetEventHandler(nullptr);
        SaveWorldIntoScene(edit_world_, GetScene());
        targetFrameRate = timing::GetTargetFrameRate();
        timing::SetTargetFrameRate(-1);
        edit_world_.Start();
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


auto EditorApp::ExtractRenderFrame(rendering::RenderFrame& frame) -> void {
  App::ExtractRenderFrame(frame);
  imgui_renderer_.ExtractFrame(frame);
}


auto EditorApp::RecordRenderFrame(rendering::RenderFrame& frame) -> void {
  App::RecordRenderFrame(frame);
  imgui_renderer_.RecordFrame(frame);
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


auto EditorApp::OnWindowFocusGain() -> void {
  if (!proj_dir_abs_.empty()) {
    asset_db_.Refresh();
  }
}


auto EditorApp::HandleBackgroundThreadException(std::exception const& ex) -> void {
  DisplayError(ex.what());
}


auto EditorApp::HandleUnknownBackgroundThreadException() -> void {
  DisplayError("Unknown error.");
}


auto EditorApp::LoadSceneIntoEditWorld(Scene const& scene, bool const clear_world) -> void {
  if (clear_world) {
    edit_world_.Clear();
  }

  // The editor saves the whole edit world back into the scene, so the group is not needed yet.
  std::ignore = edit_scene_loader_.Load(scene);
}


auto EditorApp::SaveWorldIntoScene(World const& world, Scene& scene) -> void {
  scene.SetEntities(
    world.GetEntities() |
    std::views::transform([](auto const& ptr) { return ptr.get(); }) |
    std::ranges::to<std::vector<Entity const*>>());
}


std::string_view const EditorApp::window_title_base_{"Mage"};
}
