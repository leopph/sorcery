#pragma once

#include "Core.hpp"
#include "job_system.hpp"
#include "object_registry.hpp"
#include "observer_ptr.hpp"
#include "resource_manager.hpp"
#include "Window.hpp"
#include "rendering/graphics.hpp"
#include "rendering/render_manager.hpp"
#include "rendering/scene_renderer.hpp"

#include <span>
#include <string_view>


namespace sorcery {
class App {
public:
  SORCERYAPI explicit App(std::span<std::string_view const> args = {});
  App(App const&) = delete;
  App(App&&) = delete;

  SORCERYAPI virtual ~App();

  auto operator=(App const&) -> void = delete;
  auto operator=(App&&) -> void = delete;

  [[nodiscard]] SORCERYAPI auto GetGraphicsDevice() -> graphics::GraphicsDevice&;
  [[nodiscard]] SORCERYAPI auto GetWindow() -> Window&;
  [[nodiscard]] SORCERYAPI auto GetSwapChain() -> graphics::SwapChain&;
  [[nodiscard]] SORCERYAPI auto GetRenderManager() -> rendering::RenderManager&;
  [[nodiscard]] SORCERYAPI auto GetSceneRenderer() -> rendering::SceneRenderer&;
  [[nodiscard]] SORCERYAPI auto GetJobSystem() -> JobSystem&;
  [[nodiscard]] SORCERYAPI auto GetObjectRegistry() -> ObjectRegistry&;
  [[nodiscard]] SORCERYAPI auto GetResourceManager() -> ResourceManager&;

  SORCERYAPI auto Run() -> void;

  [[nodiscard]] SORCERYAPI static auto Instance() -> App&;

protected:
  SORCERYAPI auto WaitRenderJob() -> void;

  SORCERYAPI virtual auto BeginFrame() -> void;
  virtual auto Update() -> void {}
  virtual auto EndFrame() -> void {}
  SORCERYAPI virtual auto PrepareRender() -> void;
  SORCERYAPI virtual auto Render() -> void;

private:
  JobSystem job_system_;
  graphics::GraphicsDevice graphics_device_;
  Window window_;
  graphics::SharedDeviceChildHandle<graphics::SwapChain> swap_chain_;
  rendering::RenderManager render_manager_;
  rendering::SceneRenderer scene_renderer_;
  ObjectRegistry object_registry_;
  ResourceManager resource_manager_;
  ObserverPtr<Job> render_job_;
  bool window_resized_{false};


  static ObserverPtr<App> instance_;
};
}
