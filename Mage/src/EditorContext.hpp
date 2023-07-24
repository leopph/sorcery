#pragma once

#include <imgui.h>

#include "ResourceDB.hpp"
#include "Scene.hpp"
#include "ObjectWrappers/ObjectWrapperManager.hpp"

#include <atomic>
#include <filesystem>
#include <memory>
#include <variant>


namespace sorcery::mage {
class Context {
  ImGuiIO& mImGuiIo;
  Scene* mScene{ nullptr };
  std::shared_ptr<ObjectWrapperManager> mWrapperManager{ ObjectWrapperManager::Create() };
  ResourceDB mResourceDB;
  Object* mSelectedObject{ nullptr };

  std::filesystem::path mProjDirAbs;

  std::atomic<bool> mBusy;

  static auto OnWindowFocusGain(Context* self) -> void;
  static auto HandleBackgroundThreadException(std::exception const& ex) -> void;
  static auto HandleUnknownBackgroundThreadException() -> void;

public:
  explicit Context(ImGuiIO& imGuiIO);
  ~Context();

  [[nodiscard]] auto GetImGuiIo() const noexcept -> ImGuiIO const&;
  [[nodiscard]] auto GetImGuiIo() noexcept -> ImGuiIO&;

  [[nodiscard]] auto GetResourceDatabase() const noexcept -> ResourceDB const&;
  [[nodiscard]] auto GetResourceDatabase() noexcept -> ResourceDB&;

  [[nodiscard]] auto GetScene() const noexcept -> Scene const*;
  [[nodiscard]] auto GetScene() noexcept -> Scene*;
  auto OpenScene(Scene& scene) -> void;

  [[nodiscard]] auto GetFactoryManager() const noexcept -> ObjectWrapperManager const&;
  [[nodiscard]] auto GetFactoryManager() noexcept -> ObjectWrapperManager&;

  [[nodiscard]] auto GetSelectedObject() const noexcept -> Object*;
  auto SetSelectedObject(Object* obj) noexcept -> void;

  [[nodiscard]] auto GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const&;

  auto OpenProject(std::filesystem::path const& targetPath) -> void;

  [[nodiscard]] auto IsEditorBusy() const noexcept -> bool;

  template<typename Callable>
  auto ExecuteInBusyEditor(Callable&& callable) -> void;


  struct BusyExecutionContext {
    ImGuiConfigFlags imGuiConfigFlagsBackup;
  };


  auto OnEnterBusyExecution() -> BusyExecutionContext;
  auto OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void;
};


template<typename Callable>
auto Context::ExecuteInBusyEditor(Callable&& callable) -> void {
  std::thread{
    [this, callable] {
      BusyExecutionContext const execContext{ OnEnterBusyExecution() };

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
