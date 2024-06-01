#pragma once

#include "observer_ptr.hpp"
#include "../editor_gui.hpp"
#include "rendering/graphics.hpp"
#include "rendering/render_manager.hpp"

#include <array>
#include <vector>


namespace sorcery::mage {
class ImGuiRenderer {
public:
  ImGuiRenderer(graphics::GraphicsDevice& device, graphics::SwapChain const& swap_chain,
                rendering::RenderManager& render_manager);

  auto UpdateFonts() -> void;

  auto ExtractDrawData() -> void;
  auto Render() -> void;

private:
  struct CmdList {
    std::vector<ImDrawCmd> CmdBuffer;
    std::vector<ImDrawIdx> IdxBuffer;
    std::vector<ImDrawVert> VtxBuffer;
    ImDrawListFlags Flags;
  };


  struct DrawData {
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVtxCount;
    std::vector<CmdList> CmdLists;
    ImVec2 DisplayPos;
    ImVec2 DisplaySize;
    ImVec2 FramebufferScale;
  };


  ObserverPtr<graphics::GraphicsDevice> device_;
  ObserverPtr<graphics::SwapChain const> swap_chain_;
  ObserverPtr<rendering::RenderManager> render_manager_;

  graphics::SharedDeviceChildHandle<graphics::PipelineState> pso_;
  graphics::UniqueSamplerHandle samp_;
  graphics::SharedDeviceChildHandle<graphics::Texture> fonts_tex_;

  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()>
  vtx_buffers_;
  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()>
  idx_buffers_;

  std::array<void*, rendering::RenderManager::GetMaxFramesInFlight()> vb_ptrs_{};
  std::array<void*, rendering::RenderManager::GetMaxFramesInFlight()> ib_ptrs_{};

  std::array<DrawData, rendering::RenderManager::GetMaxFramesInFlight()> draw_data_{};
};
}
