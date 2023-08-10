#pragma once

#include <imgui.h>

#include "ResourceDB.hpp"
#include "Scene.hpp"

#include <atomic>
#include <filesystem>
#include <memory>
#include <variant>


namespace sorcery::mage {
class Application {
  ImGuiIO& mImGuiIo;
  Scene* mScene{new Scene{}};
  Object* mSelectedObject{nullptr};
  ResourceDB mResourceDB{mSelectedObject};

  std::filesystem::path mProjDirAbs;

  std::atomic<bool> mBusy;
  bool mIsInDarkMode{true};

  static std::string_view const WINDOW_TITLE_BASE;

  static auto OnWindowFocusGain(Application* self) -> void;
  static auto HandleBackgroundThreadException(std::exception const& ex) -> void;
  static auto HandleUnknownBackgroundThreadException() -> void;

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

  [[nodiscard]] auto GetScene() const noexcept -> Scene&;
  auto OpenScene(Scene& scene) -> void;
  auto SaveCurrentSceneToFile() -> void;

  [[nodiscard]] auto GetSelectedObject() const noexcept -> Object*;
  auto SetSelectedObject(Object* obj) noexcept -> void;

  [[nodiscard]] auto GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const&;

  auto OpenProject(std::filesystem::path const& targetPath) -> void;

  [[nodiscard]] auto IsEditorBusy() const noexcept -> bool;

  [[nodiscard]] auto IsGuiDarkMode() const noexcept -> bool;
  auto SetGuiDarkMode(bool darkMode) noexcept -> void;

private:
  template<typename Callable>
  auto ExecuteInBusyEditor(Callable&& callable) -> void;


  struct BusyExecutionContext {
    ImGuiConfigFlags imGuiConfigFlagsBackup;
  };


  auto OnEnterBusyExecution() -> BusyExecutionContext;
  auto OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void;
};


template<typename Callable>
auto Application::ExecuteInBusyEditor(Callable&& callable) -> void {
  std::thread{
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
  }.detach();
}
}
