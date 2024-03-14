#pragma once

#include "observer_ptr.hpp"
#include "rendering/graphics.hpp"
#include "rendering/render_manager.hpp"
#include "Window.hpp"

#include <imgui.h>

#include <array>


namespace sorcery::mage {
class ImGuiRenderer {
public:
  ImGuiRenderer(graphics::GraphicsDevice& device, Window const& window, rendering::RenderManager& render_manager);

  auto Render(ImDrawData* draw_data) -> void;

private:
  ObserverPtr<graphics::GraphicsDevice> device_;
  ObserverPtr<Window const> window_;
  ObserverPtr<rendering::RenderManager> render_manager_;

  graphics::SharedDeviceChildHandle<graphics::PipelineState> pso_;
  graphics::UniqueSamplerHandle samp_;
  graphics::SharedDeviceChildHandle<graphics::Texture> fonts_tex_;

  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()> vtx_buffers_;
  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()> idx_buffers_;

  std::array<void*, rendering::RenderManager::GetMaxFramesInFlight()> vb_ptrs_{};
  std::array<void*, rendering::RenderManager::GetMaxFramesInFlight()> ib_ptrs_{};
};
}
