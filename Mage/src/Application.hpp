#pragma once

#include <imgui.h>

#include "Event.hpp"
#include "job_system.hpp"
#include "engine_context.hpp"
#include "ResourceDB.hpp"
#include "Scene.hpp"

#include <atomic>
#include <filesystem>
#include <memory>
#include <type_traits>
#include <variant>


namespace sorcery::mage {
class Application {
public:
  explicit Application(ImGuiIO& imGuiIO);
  Application(Application const&) = delete;
  Application(Application&&) = delete;

  ~Application();

  auto operator=(Application const&) -> void = delete;
  auto operator=(Application&&) -> void = delete;

  [[nodiscard]] auto GetImGuiIo() const noexcept -> ImGuiIO const&;
  [[nodiscard]] auto GetImGuiIo() noexcept -> ImGuiIO&;

  [[nodiscard]] auto GetResourceDatabase() const noexcept -> ResourceDB const&;
  [[nodiscard]] auto GetResourceDatabase() noexcept -> ResourceDB&;

  auto OpenScene(Guid const& guid) -> void;
  auto OpenNewScene() -> void;
  auto SaveCurrentSceneToFile() -> void;
  auto CloseScene() -> void;
  [[nodiscard]] auto GetScene() const noexcept -> Scene&;

  [[nodiscard]] auto GetSelectedObject() const noexcept -> Object*;
  auto SetSelectedObject(Object* obj) noexcept -> void;

  [[nodiscard]] auto GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const&;

  auto OpenProject(std::filesystem::path const& targetPath) -> void;

  [[nodiscard]] auto IsEditorBusy() const noexcept -> bool;

  [[nodiscard]] auto IsGuiDarkMode() const noexcept -> bool;
  auto SetGuiDarkMode(bool darkMode) noexcept -> void;

  template<typename Callable>
  auto ExecuteInBusyEditor(Callable const& callable) -> void;

private:
  struct BusyExecutionContext {
    ImGuiConfigFlags imGuiConfigFlagsBackup;
  };


  auto OnEnterBusyExecution() -> BusyExecutionContext;
  auto OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void;

  auto OnWindowFocusGain() -> void;
  static auto HandleBackgroundThreadException(std::exception const& ex) -> void;
  static auto HandleUnknownBackgroundThreadException() -> void;

  ImGuiIO& imgui_io_;
  std::unique_ptr<Scene> temp_scene_owner_;
  Scene* scene_{nullptr};
  Object* selected_object_{nullptr};
  ResourceDB resource_db_{selected_object_};

  std::filesystem::path proj_dir_abs_;

  std::atomic<bool> busy_;
  bool dark_mode_{true};

  EventListenerHandle<void> window_focus_gain_listener_{};

  static std::string_view const WINDOW_TITLE_BASE;
};


template<typename Callable>
auto Application::ExecuteInBusyEditor(Callable const& callable) -> void {
  auto const job_func{
    [this, callable] {
      BusyExecutionContext const execContext{OnEnterBusyExecution()};

      try {
        std::invoke(callable);
      } catch (std::exception const& ex) {
        HandleBackgroundThreadException(ex);
      } catch (...) {
        HandleUnknownBackgroundThreadException();
      }

      OnFinishBusyExecution(execContext);
    }
  };

  g_engine_context.job_system->Run(g_engine_context.job_system->CreateJob([](void const* const data_ptr) {
    (*static_cast<decltype(job_func)* const>(data_ptr))();
  }, job_func));
}
}
