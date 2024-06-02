#include "app.hpp"

#include "MemoryAllocation.hpp"
#include "Platform.hpp"
#include "Timing.hpp"

#include <stdexcept>


namespace sorcery {
App::App(std::span<std::string_view const> const args) :
  graphics_device_{
#ifndef NDEBUG
    true
#else
    false
#endif
  },
  swap_chain_{
    graphics_device_.CreateSwapChain(graphics::SwapChainDesc{
      0, 0, 2, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_USAGE_RENDER_TARGET_OUTPUT, DXGI_SCALING_STRETCH
    }, static_cast<HWND>(window_.GetNativeHandle()))
  },
  render_manager_{graphics_device_},
  scene_renderer_{window_, graphics_device_, render_manager_},
  resource_manager_{job_system_} {
  if (instance_) {
    throw std::logic_error{"App already exists!"};
  }

  instance_.Reset(this);

  resource_manager_.CreateDefaultResources();

  window_.OnWindowSize.add_listener([this](Extent2D<unsigned>) {
    window_resized_ = true;
  });

  timing::OnApplicationStart();
}


App::~App() {
  WaitRenderJob();
  graphics_device_.WaitIdle();
}


auto App::GetGraphicsDevice() -> graphics::GraphicsDevice& {
  return graphics_device_;
}


auto App::GetWindow() -> Window& {
  return window_;
}


auto App::GetSwapChain() -> graphics::SwapChain& {
  return *swap_chain_;
}


auto App::GetRenderManager() -> rendering::RenderManager& {
  return render_manager_;
}


auto App::GetSceneRenderer() -> rendering::SceneRenderer& {
  return scene_renderer_;
}


auto App::GetJobSystem() -> JobSystem& {
  return job_system_;
}


auto App::GetResourceManager() -> ResourceManager& {
  return resource_manager_;
}


auto App::Run() -> void {
  while (!IsQuitSignaled()) {
    ProcessEvents();
    BeginFrame();

    try {
      Update();
    } catch (std::runtime_error const& err) {
      DisplayError(err.what());
    }

    EndFrame();

    if (render_job_) {
      job_system_.Wait(render_job_.Get());
    }

    if (window_resized_) {
      graphics_device_.WaitIdle();
      graphics_device_.ResizeSwapChain(*swap_chain_, 0, 0);
      window_resized_ = false;
    }

    GetSingleFrameLinearMemory().Clear();

    scene_renderer_.ExtractCurrentState();
    PrepareRender();

    render_job_.Reset(job_system_.CreateJob([this] {
      scene_renderer_.Render();
      Render();
      swap_chain_->Present();
      render_manager_.EndFrame();
    }));

    job_system_.Run(render_job_.Get());

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
  if (render_job_) {
    job_system_.Wait(render_job_.Get());
  }
}


ObserverPtr<App> App::instance_{};
}
