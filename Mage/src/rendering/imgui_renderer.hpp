#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "observer_ptr.hpp"
#include "../gui_helpers.hpp"
#include "rendering/config.hpp"
#include "wand/wand.hpp"


namespace sorcery {
namespace rendering {
class RenderManager;
class RenderFrame;
class TextureResolver;
}


namespace mage {
class ImGuiTextureReferences;


class ImGuiRenderer {
public:
  ImGuiRenderer(
    wand::GraphicsDevice& device,
    wand::SwapChain const& swap_chain,
    rendering::RenderManager& render_manager,
    rendering::TextureResolver& tex_resolver,
    ImGuiTextureReferences const& tex_refs
  );
  ImGuiRenderer(ImGuiRenderer const& other) = delete;
  ImGuiRenderer(ImGuiRenderer&& other) noexcept = delete;

  ~ImGuiRenderer() = default;

  auto operator=(ImGuiRenderer const& other) -> ImGuiRenderer& = delete;
  auto operator=(ImGuiRenderer&& other) noexcept -> ImGuiRenderer& = delete;

  auto UpdateFonts() -> void;

  auto ExtractFrame(rendering::RenderFrame& frame) -> void;
  auto RecordFrame(rendering::RenderFrame& frame) -> void;

private:
  struct CmdList {
    std::vector<ImDrawCmd> cmd_buffer;
    std::vector<ImDrawIdx> idx_buffer;
    std::vector<ImDrawVert> vtx_buffer;
    ImDrawListFlags flags;
  };


  struct DrawData {
    bool valid;
    int cmd_lists_count;
    int total_idx_count;
    int total_vtx_count;
    std::vector<CmdList> cmd_lists;
    ImVec2 display_pos;
    ImVec2 display_size;
    ImVec2 framebuffer_scale;
    std::vector<wand::SharedDeviceHandle<wand::Texture>> textures;
  };


  [[nodiscard]]
  auto ResolveImGuiTexture(ImTextureID id, std::uint32_t frame_idx) const -> ObserverPtr<wand::Texture>;


  ObserverPtr<wand::GraphicsDevice> device_;
  ObserverPtr<wand::SwapChain const> swap_chain_;
  ObserverPtr<rendering::RenderManager> render_manager_;
  ObserverPtr<rendering::TextureResolver> tex_resolver_;
  ObserverPtr<ImGuiTextureReferences const> tex_refs_;

  wand::SharedDeviceHandle<wand::PipelineState> pso_;
  wand::UniqueSamplerHandle samp_;
  wand::SharedDeviceHandle<wand::Texture> fonts_tex_;

  std::array<wand::SharedDeviceHandle<wand::BufferView>, rendering::kFramesInFlight>
  vtx_buffers_;
  std::array<wand::SharedDeviceHandle<wand::Buffer>, rendering::kFramesInFlight>
  idx_buffers_;

  std::array<void*, rendering::kFramesInFlight> vb_ptrs_{};
  std::array<void*, rendering::kFramesInFlight> ib_ptrs_{};

  std::array<DrawData, rendering::kFramesInFlight> draw_data_{};

  ImTextureID const static kFontTexId;
};
}
}
