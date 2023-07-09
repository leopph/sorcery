#pragma once

#include <imgui.h>

#include "ResourceManager.hpp"
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
  ResourceManager mResourceManager{ mWrapperManager, this };
  Object* mSelectedObject{ nullptr };

  static inline std::filesystem::path ASSET_DIR_REL{ "Assets" };
  static inline std::filesystem::path CACHE_DIR_REL{ "Cache" };
  static inline std::filesystem::path ASSET_FILE_EXT{ ".leopphasset" };

  std::filesystem::path mProjDirAbs;
  std::filesystem::path mAssetDirAbs;
  std::filesystem::path mCacheDirAbs;

  std::atomic<bool> mBusy;

  static auto OnWindowFocusGain(Context* self) -> void;
  static auto HandleBackgroundThreadException(std::exception const& ex) -> void;
  static auto HandleUnknownBackgroundThreadException() -> void;

public:
  explicit Context(ImGuiIO& imGuiIO);
  ~Context();

  [[nodiscard]] auto GetImGuiIo() const noexcept -> ImGuiIO const&;
  [[nodiscard]] auto GetImGuiIo() noexcept -> ImGuiIO&;

  [[nodiscard]] auto GetResources() const noexcept -> ResourceManager const&;
  [[nodiscard]] auto GetResources() noexcept -> ResourceManager&;

  [[nodiscard]] auto GetScene() const noexcept -> Scene const*;
  [[nodiscard]] auto GetScene() noexcept -> Scene*;
  auto OpenScene(Scene& scene) -> void;

  [[nodiscard]] auto GetFactoryManager() const noexcept -> ObjectWrapperManager const&;
  [[nodiscard]] auto GetFactoryManager() noexcept -> ObjectWrapperManager&;

  [[nodiscard]] auto GetSelectedObject() const noexcept -> Object*;
  auto SetSelectedObject(Object* obj) noexcept -> void;

  [[nodiscard]] auto GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const&;
  [[nodiscard]] auto GetAssetDirectoryAbsolute() const noexcept -> std::filesystem::path const&;
  [[nodiscard]] auto GetCacheDirectoryAbsolute() const noexcept -> std::filesystem::path const&;

  [[nodiscard]] static auto GetAssetDirectoryProjectRootRelative() noexcept -> std::filesystem::path const&;
  [[nodiscard]] static auto GetCacheDirectoryProjectRootRelative() noexcept -> std::filesystem::path const&;

  [[nodiscard]] inline static auto GetAssetFileExtension() noexcept -> std::filesystem::path const&;

  auto OpenProject(std::filesystem::path const& targetPath) -> void;

  [[nodiscard]] auto IsEditorBusy() const noexcept -> bool;

  template<typename Callable>
  auto ExecuteInBusyEditor(Callable&& callable) -> void;

  auto CreateMetaFileForRegisteredAsset(Object const& asset) const -> void;
  auto SaveRegisteredNativeAsset(NativeAsset const& asset) const -> void;


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


inline auto Context::GetAssetFileExtension() noexcept -> std::filesystem::path const& {
  return ASSET_FILE_EXT;
}
}
