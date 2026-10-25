#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "AssetDB.hpp"
#include "gui_helpers.hpp"
#include "imgui_texture_references.hpp"
#include "MainMenuBar.hpp"
#include "drawers/editor_drawer_registry.hpp"
#include "rendering/imgui_renderer.hpp"
#include "sorcery/event.hpp"
#include "sorcery/world.hpp"
#include "sorcery/assets/Scene.hpp"
#include "sorcery/host/app.hpp"
#include "windows/EntityHierarchyWindow.hpp"
#include "windows/GameViewWindow.hpp"
#include "windows/ProjectWindow.hpp"
#include "windows/PropertiesWindow.hpp"
#include "windows/SceneViewWindow.hpp"


namespace sorcery::mage {
class EditorApp final : public App {
public:
  explicit EditorApp(CommandLineArgs const& args);
  EditorApp(EditorApp const&) = delete;
  EditorApp(EditorApp&&) = delete;

  ~EditorApp() override;

  auto operator=(EditorApp const&) -> void = delete;
  auto operator=(EditorApp&&) -> void = delete;

  [[nodiscard]] auto GetImGuiIo() const noexcept -> ImGuiIO const&;
  [[nodiscard]] auto GetImGuiIo() noexcept -> ImGuiIO&;

  [[nodiscard]] auto GetAssetDatabase() const noexcept -> AssetDB const&;
  [[nodiscard]] auto GetAssetDatabase() noexcept -> AssetDB&;

  auto OpenScene(AssetId const& asset_id) -> void;
  auto OpenNewScene() -> void;
  auto SaveCurrentSceneToFile() -> void;
  [[nodiscard]] auto GetScene() const noexcept -> Scene&;

  [[nodiscard]] auto GetSelectedObject() const noexcept -> ObjectPtr<Object>;
  auto SetSelectedObject(ObjectPtr<Object> obj) noexcept -> void;

  [[nodiscard]] auto GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const&;

  auto OpenProject(std::filesystem::path const& targetPath) -> void;

  [[nodiscard]] auto IsEditorBusy() const noexcept -> bool;

  [[nodiscard]] auto IsGuiDarkMode() const noexcept -> bool;
  auto SetGuiDarkMode(bool darkMode) noexcept -> void;

  template<typename Callable>
  auto ExecuteInBusyEditor(Callable&& callable) -> void;

protected:
  [[nodiscard]]
  auto GetRenderedWorld() -> World& override;

  auto BeginFrame() -> void override;
  auto Update() -> void override;
  auto EndFrame() -> void override;
  auto ExtractRenderFrame(rendering::RenderFrame& frame) -> void override;
  auto RecordRenderFrame(rendering::RenderFrame& frame) -> void override;

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

  AssetRef<Scene> scene_;
  ObjectPtr<Object> selected_object_{nullptr};
  AssetDB asset_db_{selected_object_};

  std::filesystem::path proj_dir_abs_;

  std::atomic<bool> busy_;
  bool dark_mode_{true};

  EventListenerHandle<void> window_focus_gain_listener_{};

  bool game_is_running_{false};

  World edit_world_;
  World play_world_;

  ImGuiTextureReferences tex_refs_;
  ImGuiRenderer imgui_renderer_{
    GetGraphicsDevice(), GetSwapChain(), GetTextureResolver(), tex_refs_
  };
  EditorDrawerRegistry drawer_registry_{tex_refs_};
  ProjectWindow project_window_{*this, asset_db_, drawer_registry_};
  SceneViewWindow scene_view_window_{tex_refs_, edit_world_};
  GameViewWindow game_view_window_{tex_refs_};
  PropertiesWindow properties_window_{*this, drawer_registry_};
  SettingsWindow editor_settings_window_{*this, scene_view_window_.GetCamera()};
  MainMenuBar main_menu_bar_{*this, editor_settings_window_, GetGameModule()};
  EntityHierarchyWindow entity_hierarchy_window_{*this, edit_world_};

  static std::string_view const window_title_base_;
};
}


#include "editor_app.inl"
