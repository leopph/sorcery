#pragma once

#include "Core.hpp"
#include "observer_ptr.hpp"
#include "rendering/graphics.hpp"
#include "rendering/render_manager.hpp"
#include "rendering/scene_renderer.hpp"
#include "ResourceManager.hpp"
#include "Window.hpp"


namespace sorcery {
class JobSystem;


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
