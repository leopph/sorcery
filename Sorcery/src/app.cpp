#include "sorcery/host/app.hpp"

#include <charconv>
#include <stdexcept>
#include <utility>

#include "wand/wand.hpp"

#include "rendering/frame_scheduler.hpp"
#include "rendering/render_instance_registry.hpp"
#include "rendering/render_resource_registry.hpp"
#include "rendering/scene_renderer.hpp"
#include "rendering/temporary_render_target_pool.hpp"
#include "sorcery/timing.hpp"
#include "sorcery/util.hpp"
#include "sorcery/world.hpp"
#include "sorcery/host/command_line_args.hpp"
#include "sorcery/host/game_module.hpp"
#include "sorcery/host/job_system.hpp"
#include "sorcery/host/object_registry.hpp"
#include "sorcery/host/platform.hpp"
#include "sorcery/host/window.hpp"
#include "sorcery/host/assets/asset_manager.hpp"
#include "sorcery/host/rendering/texture_resolver.hpp"


namespace sorcery {
struct App::Data {
  explicit Data(CommandLineArgs const& args) :
    job_system{
      [&args] {
        unsigned thread_count{0};

        if (auto const threads_arg = args.GetArg("threads")) {
          std::from_chars(threads_arg->data(), threads_arg->data() + threads_arg->size(), thread_count);
        }

        return thread_count;
      }()
    },
    graphics_device{
#ifndef NDEBUG
      true,
#else
      false,
#endif
      args.HasArg("swrendering")
    } {}


  JobSystem job_system;
  wand::GraphicsDevice graphics_device;
  Window window;
  wand::SharedDeviceHandle<wand::SwapChain> swap_chain{
    graphics_device.CreateSwapChain(wand::SwapChainDesc{
      0, 0, 2, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_USAGE_RENDER_TARGET_OUTPUT, DXGI_SCALING_STRETCH
    }, static_cast<HWND>(window.GetNativeHandle()))
  };
  rendering::FrameScheduler frame_scheduler{graphics_device};
  rendering::TemporaryRenderTargetPool rt_pool{graphics_device};
  GameModule game_module;
  ObjectRegistry object_registry;
  rendering::RenderResourceRegistry render_resource_registry{graphics_device, object_registry};
  rendering::RenderInstanceRegistry render_instance_registry{object_registry};
  rendering::TextureResolver texture_resolver{graphics_device, render_resource_registry};
  rendering::SceneRenderer scene_renderer{
    window, graphics_device, rt_pool, render_resource_registry, render_instance_registry, texture_resolver
  };
  AssetManager asset_manager{job_system};
  ObserverPtr<Job> render_job;
};


App::App(CommandLineArgs const& args) :
  data_{std::make_unique<Data>(args)} {
  if (instance_) {
    throw std::logic_error{"App already exists!"};
  }

  instance_.Reset(this);

  data_->asset_manager.CreateDefaultAssets();

  data_->window.OnWindowSize.add_listener([this](Extent2D<unsigned>) {
    window_resized_ = true;
  });

  timing::OnApplicationStart();
}


App::~App() {
  WaitRenderJobNoThrow();
  data_->graphics_device.WaitIdle();
}


auto App::GetJobSystem() -> JobSystem& {
  return data_->job_system;
}


auto App::GetGraphicsDevice() -> wand::GraphicsDevice& {
  return data_->graphics_device;
}


auto App::GetWindow() -> Window& {
  return data_->window;
}


auto App::GetSwapChain() -> wand::SwapChain& {
  return *data_->swap_chain;
}


auto App::GetSceneRenderer() -> rendering::SceneRenderer& {
  return data_->scene_renderer;
}


auto App::GetObjectRegistry() -> ObjectRegistry& {
  return data_->object_registry;
}


auto App::GetAssetManager() -> AssetManager& {
  return data_->asset_manager;
}


auto App::Run() -> void {
  while (!IsQuitSignaled()) {
    BeginFrame();

    try {
      Update();
    } catch (std::runtime_error const& err) {
      DisplayError(err.what());
    }

    EndFrame();

    WaitRenderJob();

    if (window_resized_) {
      if (auto const [width, height]{data_->window.GetClientAreaSize()}; width != 0 && height != 0) {
        data_->graphics_device.WaitIdle();
        data_->graphics_device.ResizeSwapChain(*data_->swap_chain, 0, 0);
      }

      window_resized_ = false;
    }

    auto& frame = data_->frame_scheduler.AcquireFrame();

    ExtractRenderFrame(frame);

    data_->render_job = data_->job_system.CreateJob([this, &frame] {
      PrepareRenderFrame(frame);
      RecordRenderFrame(frame);
      data_->frame_scheduler.SubmitFrame(frame);
      data_->graphics_device.Present(*data_->swap_chain);
      data_->render_instance_registry.CollectGarbage(100uz);
      data_->render_resource_registry.CollectGarbage(100uz);
      data_->rt_pool.CollectGarbage(frame);
    });

    data_->job_system.Submit(data_->render_job);

    timing::OnFrameEnd();
  }
}


auto App::Instance() -> App& {
  if (!instance_) {
    throw std::logic_error{"App does not exist!"};
  }

  return *instance_;
}


auto App::GetTextureResolver() -> rendering::TextureResolver& {
  return data_->texture_resolver;
}


auto App::GetGameModule() -> GameModule& {
  return data_->game_module;
}


auto App::WaitRenderJob() -> void {
  // Exchange with null so a failed job is only consumed once
  if (auto const job = std::exchange(data_->render_job, nullptr)) {
    data_->job_system.Wait(job);
  }
}


auto App::WaitRenderJobNoThrow() noexcept -> void {
  // Exchange with null so a failed job is only consumed once
  if (auto const job = std::exchange(data_->render_job, nullptr)) {
    if (auto const ex_ptr = data_->job_system.TryWait(job)) {
      LogException("Render job failed during shutdown", ex_ptr);
    }
  }
}


auto App::BeginFrame() -> void {
  ProcessEvents();
}


auto App::ExtractRenderFrame(rendering::RenderFrame& frame) -> void {
  data_->scene_renderer.ExtractFrame(frame, GetRenderedWorld().GetRenderWorld());
}


auto App::PrepareRenderFrame(rendering::RenderFrame& frame) -> void {
  data_->scene_renderer.PrepareFrame(frame);
}


auto App::RecordRenderFrame(rendering::RenderFrame& frame) -> void {
  data_->scene_renderer.RecordFrame(frame);
}


ObserverPtr<App> App::instance_{};
}
