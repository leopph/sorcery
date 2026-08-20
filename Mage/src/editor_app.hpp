#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "app.hpp"
#include "gui_helpers.hpp"
#include "Event.hpp"
#include "MainMenuBar.hpp"
#include "ResourceDB.hpp"
#include "Scene.hpp"
#include "drawers/editor_drawer_registry.hpp"
#include "rendering/imgui_renderer.hpp"
#include "windows/EntityHierarchyWindow.hpp"
#include "windows/GameViewWindow.hpp"
#include "windows/ProjectWindow.hpp"
#include "windows/PropertiesWindow.hpp"
#include "windows/SceneViewWindow.hpp"


namespace sorcery::mage {
class EditorApp final : public App {
public:
  explicit EditorApp(std::span<std::string_view const> args = {});
  EditorApp(EditorApp const&) = delete;
  EditorApp(EditorApp&&) = delete;

  ~EditorApp() override;

  auto operator=(EditorApp const&) -> void = delete;
  auto operator=(EditorApp&&) -> void = delete;

  auto BeginFrame() -> void override;
  auto Update() -> void override;
  auto EndFrame() -> void override;
  auto PrepareRender() -> void override;
  auto Render() -> void override;

  [[nodiscard]] auto GetImGuiIo() const noexcept -> ImGuiIO const&;
  [[nodiscard]] auto GetImGuiIo() noexcept -> ImGuiIO&;

  [[nodiscard]] auto GetResourceDatabase() const noexcept -> ResourceDB const&;
  [[nodiscard]] auto GetResourceDatabase() noexcept -> ResourceDB&;

  auto OpenScene(ResourceId const& res_id) -> void;
  auto OpenNewScene() -> void;
  auto SaveCurrentSceneToFile() -> void;
  [[nodiscard]] auto GetScene() const noexcept -> Scene&;

  [[nodiscard]] auto GetSelectedObject() const noexcept -> Object*;
  auto SetSelectedObject(Object* obj) noexcept -> void;

  [[nodiscard]] auto GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const&;

  auto OpenProject(std::filesystem::path const& targetPath) -> void;

  [[nodiscard]] auto IsEditorBusy() const noexcept -> bool;

  [[nodiscard]] auto IsGuiDarkMode() const noexcept -> bool;
  auto SetGuiDarkMode(bool darkMode) noexcept -> void;

  template<typename Callable>
  auto ExecuteInBusyEditor(Callable&& callable) -> void;

private:
  struct BusyExecutionContext {
    ImGuiConfigFlags imGuiConfigFlagsBackup;
  };


  auto OnEnterBusyExecution() -> BusyExecutionContext;
  auto OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void;

  auto OnWindowFocusGain() -> void;
  static auto HandleBackgroundThreadException(std::exception const& ex) -> void;
  static auto HandleUnknownBackgroundThreadException() -> void;

  ObserverPtr<ImGuiContext> imgui_ctx_;
  ObserverPtr<ImGuiIO> imgui_io_;
  std::string imgui_io_ini_path_;

  ObserverPtr<Scene> scene_;
  Object* selected_object_{nullptr};
  ResourceDB resource_db_{selected_object_};

  std::filesystem::path proj_dir_abs_;

  std::atomic<bool> busy_;
  bool dark_mode_{true};

  EventListenerHandle<void> window_focus_gain_listener_{};

  bool game_is_running_{false};

  ImGuiRenderer imgui_renderer_{GetGraphicsDevice(), GetSwapChain(), GetRenderManager()};

  EditorDrawerRegistry drawer_registry_;

  ProjectWindow project_window_{*this, resource_db_};
  SceneViewWindow scene_view_window_;
  GameViewWindow game_view_window_;
  PropertiesWindow properties_window_{*this, drawer_registry_};
  SettingsWindow editor_settings_window_{*this, scene_view_window_.GetCamera()};
  MainMenuBar main_menu_bar_{*this, editor_settings_window_};
  EntityHierarchyWindow entity_hierarchy_window_{*this};

  static std::string_view const window_title_base_;
};
}


#include "editor_app.inl"
