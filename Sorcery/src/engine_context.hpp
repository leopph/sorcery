#pragma once

#include "Core.hpp"
#include "observer_ptr.hpp"
#include "rendering/graphics.hpp"
#include "rendering/render_manager.hpp"
#include "rendering/scene_renderer.hpp"
#include "Window.hpp"

namespace sorcery {
struct EngineContext {
  ObserverPtr<graphics::GraphicsDevice> graphics_device;
  ObserverPtr<rendering::RenderManager> render_manager;
  ObserverPtr<rendering::SceneRenderer> scene_renderer;
  ObserverPtr<Window> window;
};

LEOPPHAPI extern EngineContext g_engine_context;
}