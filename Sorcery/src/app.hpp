#pragma once

#include "Core.hpp"
#include "job_system.hpp"
#include "observer_ptr.hpp"
#include "ResourceManager.hpp"
#include "Window.hpp"
#include "rendering/graphics.hpp"
#include "rendering/render_manager.hpp"
#include "rendering/scene_renderer.hpp"

#include <span>
#include <string_view>


namespace sorcery {
class App {
public:
  LEOPPHAPI explicit App(std::span<std::string_view const> args = {});
  App(App const&) = delete;
  App(App&&) = delete;

  LEOPPHAPI virtual ~App();

  auto operator=(App const&) -> void = delete;
  auto operator=(App&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetGraphicsDevice() -> graphics::GraphicsDevice&;
  [[nodiscard]] LEOPPHAPI auto GetWindow() -> Window&;
  [[nodiscard]] LEOPPHAPI auto GetSwapChain() -> graphics::SwapChain&;
  [[nodiscard]] LEOPPHAPI auto GetRenderManager() -> rendering::RenderManager&;
  [[nodiscard]] LEOPPHAPI auto GetSceneRenderer() -> rendering::SceneRenderer&;
  [[nodiscard]] LEOPPHAPI auto GetJobSystem() -> JobSystem&;
  [[nodiscard]] LEOPPHAPI auto GetResourceManager() -> ResourceManager&;

  LEOPPHAPI auto Run() -> void;

  [[nodiscard]] LEOPPHAPI static auto Instance() -> App&;

protected:
  LEOPPHAPI auto WaitRenderJob() -> void;

  virtual auto BeginFrame() -> void {}
  virtual auto Update() -> void {}
  virtual auto EndFrame() -> void {}
  virtual auto PrepareRender() -> void {}
  virtual auto Render() -> void {}

private:
  JobSystem job_system_;
  graphics::GraphicsDevice graphics_device_;
  Window window_;
  graphics::SharedDeviceChildHandle<graphics::SwapChain> swap_chain_;
  rendering::RenderManager render_manager_;
  rendering::SceneRenderer scene_renderer_;
  ResourceManager resource_manager_;
  bool window_resized_{false};
  ObserverPtr<Job> render_job_;


  static ObserverPtr<App> instance_;
};
}
