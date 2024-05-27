#pragma once

#include "Core.hpp"
#include "observer_ptr.hpp"


namespace sorcery {
class Window;
class JobSystem;
class ResourceManager;


namespace graphics {
class GraphicsDevice;
}


namespace rendering {
class RenderManager;
class SceneRenderer;
}


struct EngineContext {
  ObserverPtr<Window> window;
  ObserverPtr<graphics::GraphicsDevice> graphics_device;
  ObserverPtr<rendering::RenderManager> render_manager;
  ObserverPtr<rendering::SceneRenderer> scene_renderer;
  ObserverPtr<JobSystem> job_system;
  ObserverPtr<ResourceManager> resource_manager;
};


LEOPPHAPI extern EngineContext g_engine_context;
}
