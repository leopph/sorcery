#pragma once

#include "observer_ptr.hpp"
#include "../gui_helpers.hpp"
#include "rendering/config.hpp"
#include "wand/wand.hpp"

#include <array>
#include <vector>


namespace sorcery {
namespace rendering {
class RenderManager;
}


namespace mage {
class ImGuiRenderer {
public:
  ImGuiRenderer(wand::GraphicsDevice& device, wand::SwapChain const& swap_chain,
                rendering::RenderManager& render_manager);

  auto UpdateFonts() -> void;

  auto ExtractDrawData(rendering::RenderFrame const& frame) -> void;
  auto Render(rendering::RenderFrame& frame) -> void;

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


  ObserverPtr<wand::GraphicsDevice> device_;
  ObserverPtr<wand::SwapChain const> swap_chain_;
  ObserverPtr<rendering::RenderManager> render_manager_;

  wand::SharedDeviceChildHandle<wand::PipelineState> pso_;
  wand::UniqueSamplerHandle samp_;
  wand::SharedDeviceChildHandle<wand::Texture> fonts_tex_;

  std::array<wand::SharedDeviceChildHandle<wand::Buffer>, rendering::kFramesInFlight>
  vtx_buffers_;
  std::array<wand::SharedDeviceChildHandle<wand::Buffer>, rendering::kFramesInFlight>
  idx_buffers_;

  std::array<void*, rendering::kFramesInFlight> vb_ptrs_{};
  std::array<void*, rendering::kFramesInFlight> ib_ptrs_{};

  std::array<DrawData, rendering::kFramesInFlight> draw_data_{};
};
}
}
