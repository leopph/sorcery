#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "Core.hpp"
#include "observer_ptr.hpp"


namespace wand {
class GraphicsDevice;
class SwapChain;
}


namespace sorcery {
namespace rendering {
class RenderManager;
class SceneRenderer;
class RenderFrame;
class TextureResolver;
}


class Window;
class JobSystem;
class ObjectRegistry;
class ResourceManager;


class App {
public:
  SORCERYAPI explicit App(std::span<std::string_view const> args = {});
  App(App const&) = delete;
  App(App&&) = delete;

  SORCERYAPI virtual ~App();

  auto operator=(App const&) -> void = delete;
  auto operator=(App&&) -> void = delete;

  [[nodiscard]] SORCERYAPI
  auto GetJobSystem() -> JobSystem&;

  [[nodiscard]] SORCERYAPI
  auto GetGraphicsDevice() -> wand::GraphicsDevice&;

  [[nodiscard]] SORCERYAPI
  auto GetWindow() -> Window&;

  [[nodiscard]] SORCERYAPI
  auto GetSwapChain() -> wand::SwapChain&;

  [[nodiscard]] SORCERYAPI
  auto GetRenderManager() -> rendering::RenderManager&;

  [[nodiscard]] SORCERYAPI
  auto GetSceneRenderer() -> rendering::SceneRenderer&;

  [[nodiscard]] SORCERYAPI
  auto GetObjectRegistry() -> ObjectRegistry&;

  [[nodiscard]] SORCERYAPI
  auto GetResourceManager() -> ResourceManager&;

  SORCERYAPI
  auto Run() -> void;

  [[nodiscard]] SORCERYAPI static
  auto Instance() -> App&;

protected:
  [[nodiscard]] SORCERYAPI
  auto GetTextureResolver() -> rendering::TextureResolver&;

  SORCERYAPI
  auto WaitRenderJob() -> void;

  SORCERYAPI virtual
  auto BeginFrame() -> void;
  virtual auto Update() -> void {}
  virtual auto EndFrame() -> void {}
  SORCERYAPI virtual
  auto ExtractRenderFrame(rendering::RenderFrame& frame) -> void;
  SORCERYAPI virtual
  auto PrepareRenderFrame(rendering::RenderFrame& frame) -> void;
  SORCERYAPI virtual
  auto RecordRenderFrame(rendering::RenderFrame& frame) -> void;

private:
  struct Data;

  std::unique_ptr<Data> data_;
  bool window_resized_{false};

  static ObserverPtr<App> instance_;
};
}
