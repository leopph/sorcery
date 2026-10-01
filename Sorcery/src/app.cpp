#include "app.hpp"

#include <charconv>
#include <stdexcept>

#include "job_system.hpp"
#include "object_registry.hpp"
#include "Platform.hpp"
#include "resource_manager.hpp"
#include "Timing.hpp"
#include "Window.hpp"
#include "rendering/frame_scheduler.hpp"
#include "rendering/render_manager.hpp"
#include "rendering/render_resource_registry.hpp"
#include "rendering/scene_renderer.hpp"
#include "wand/wand.hpp"


namespace sorcery {
struct App::Data {
  explicit Data(std::span<std::string_view const> const args) :
    job_system{
      [args] {
        unsigned thread_count{0};

        for (auto const arg : args) {
          if (arg.starts_with("-threads=")) {
            auto const thread_count_sv{arg.substr(9)};
            if (std::from_chars(thread_count_sv.data(), thread_count_sv.data() + thread_count_sv.size(),
                  thread_count).ec
                == std::errc{}) {
              break;
            }
          }
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
      std::ranges::any_of(args, [](std::string_view const arg) {
        return arg == "-swrendering";
      })
    } {}


  JobSystem job_system;
  wand::GraphicsDevice graphics_device;
  Window window;
  wand::SharedDeviceChildHandle<wand::SwapChain> swap_chain{
    graphics_device.CreateSwapChain(wand::SwapChainDesc{
      0, 0, 2, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_USAGE_RENDER_TARGET_OUTPUT, DXGI_SCALING_STRETCH
    }, static_cast<HWND>(window.GetNativeHandle()))
  };
  rendering::FrameScheduler frame_scheduler{graphics_device};
  rendering::RenderManager render_manager{graphics_device};
  ObjectRegistry object_registry;
  rendering::RenderResourceRegistry render_resource_registry{graphics_device, object_registry};
  rendering::SceneRenderer scene_renderer{window, graphics_device, render_manager, render_resource_registry};
  ResourceManager resource_manager{job_system};
  ObserverPtr<Job> render_job;
};


App::App(std::span<std::string_view const> const args) :
  data_{std::make_unique<Data>(args)} {
  if (instance_) {
    throw std::logic_error{"App already exists!"};
  }

  instance_.Reset(this);

  data_->resource_manager.CreateDefaultResources();

  data_->window.OnWindowSize.add_listener([this](Extent2D<unsigned>) {
    window_resized_ = true;
  });

  timing::OnApplicationStart();
}


App::~App() {
  WaitRenderJob();
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


auto App::GetRenderManager() -> rendering::RenderManager& {
  return data_->render_manager;
}


auto App::GetSceneRenderer() -> rendering::SceneRenderer& {
  return data_->scene_renderer;
}


auto App::GetObjectRegistry() -> ObjectRegistry& {
  return data_->object_registry;
}


auto App::GetResourceManager() -> ResourceManager& {
  return data_->resource_manager;
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

    if (data_->render_job) {
      data_->job_system.Wait(data_->render_job);
    }

    if (window_resized_) {
      if (auto const [width, height]{data_->window.GetClientAreaSize()}; width != 0 && height != 0) {
        data_->graphics_device.WaitIdle();
        data_->graphics_device.ResizeSwapChain(*data_->swap_chain, 0, 0);
      }

      window_resized_ = false;
    }

    auto& frame = data_->frame_scheduler.AcquireFrame();

    PrepareRender(frame);

    data_->render_job = data_->job_system.CreateJob([this, &frame] {
      frame.RecordUploads();
      RecordRender(frame);
      data_->frame_scheduler.SubmitFrame(frame);
      data_->graphics_device.Present(*data_->swap_chain);
      data_->render_resource_registry.CollectGarbage(100uz);
      data_->render_manager.EndFrame();
    });

    data_->job_system.Run(data_->render_job);

    timing::OnFrameEnd();
  }
}


auto App::Instance() -> App& {
  if (!instance_) {
    throw std::logic_error{"App does not exist!"};
  }

  return *instance_;
}


auto App::WaitRenderJob() -> void {
  if (data_->render_job) {
    data_->job_system.Wait(data_->render_job);
  }
}


auto App::BeginFrame() -> void {
  ProcessEvents();
}


auto App::PrepareRender(rendering::RenderFrame& frame) -> void {
  data_->scene_renderer.ExtractCurrentState(frame);
}


auto App::RecordRender(rendering::RenderFrame& frame) -> void {
  data_->scene_renderer.Record(frame);
}


ObserverPtr<App> App::instance_{};
}
