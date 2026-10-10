// Based on Dear ImGui's DX12 renderer backend
// Adapted to Sorcery's GraphicsDevice

#include "imgui_renderer.hpp"

#include <bit>
#include <cassert>
#include <limits>

#include "object_ptr.hpp"
#include "texture_resolver.hpp"
#include "../imgui_texture_references.hpp"
#include "rendering/render_frame.hpp"
#include "rendering/render_target.hpp"
#include "resources/Cubemap.hpp"
#include "resources/Texture2D.hpp"
#include "shaders/imgui_shader_interop.h"

#ifndef NDEBUG
#include "shaders/generated/Debug/imgui_ps.h"
#include "shaders/generated/Debug/imgui_vs.h"
#else
#include "shaders/generated/Release/imgui_ps.h"
#include "shaders/generated/Release/imgui_vs.h"
#endif


namespace sorcery::mage {
ImGuiRenderer::ImGuiRenderer(
  wand::GraphicsDevice& device,
  wand::SwapChain const& swap_chain,
  rendering::TextureResolver& tex_resolver,
  ImGuiTextureReferences const& tex_refs
) :
  device_{&device},
  swap_chain_{&swap_chain},
  tex_resolver_{&tex_resolver},
  tex_refs_{&tex_refs} {
  auto& io{ImGui::GetIO()};
  io.BackendRendererName = "Sorcery ImGui Renderer";
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

  pso_ = device_->CreatePipelineState(wand::PipelineDesc{
    .vs = CD3DX12_SHADER_BYTECODE{g_imgui_vs_bytes, ARRAYSIZE(g_imgui_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{g_imgui_ps_bytes, ARRAYSIZE(g_imgui_ps_bytes)},
    .blend_state = CD3DX12_BLEND_DESC{
      D3D12_BLEND_DESC{
        FALSE, false,
        {
          {
            TRUE, FALSE, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE,
            D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD, D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
          }
        }
      }
    },
    .depth_stencil_state = CD3DX12_DEPTH_STENCIL_DESC1{
      FALSE, {}, {}, FALSE, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
    },
    .rasterizer_state = CD3DX12_RASTERIZER_DESC{
      D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE, D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
      D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
    },
    .rt_formats = CD3DX12_RT_FORMAT_ARRAY{D3D12_RT_FORMAT_ARRAY{{DXGI_FORMAT_R8G8B8A8_UNORM}, 1}},
  }, sizeof(ImGuiDrawParams) / 4);

  samp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 0, D3D12_COMPARISON_FUNC_NEVER, {0.0f, 0.0f, 0.0f, 0.0f}, 0,
    std::numeric_limits<float>::max(),
  });

  UpdateFonts();
}


auto ImGuiRenderer::UpdateFonts() -> void {
  auto& fonts = *ImGui::GetIO().Fonts;
  fonts.Build();
  fonts.SetTexID(kFontTexId);
  fonts_dirty_ = true;
}


auto ImGuiRenderer::ExtractFrame(rendering::RenderFrame& frame) -> void {
  auto const& imgui_draw_data = *ImGui::GetDrawData();
  auto& draw_data = draw_data_[frame.GetIndex()];

  draw_data.valid = imgui_draw_data.Valid;
  draw_data.cmd_lists_count = imgui_draw_data.CmdListsCount;
  draw_data.total_idx_count = imgui_draw_data.TotalIdxCount;
  draw_data.total_vtx_count = imgui_draw_data.TotalVtxCount;
  draw_data.display_pos = imgui_draw_data.DisplayPos;
  draw_data.display_size = imgui_draw_data.DisplaySize;
  draw_data.framebuffer_scale = imgui_draw_data.FramebufferScale;

  draw_data.cmd_lists.clear();

  // We never shrink the CmdLists vector to keep the vectors inside the CmdLists alive and prevent unnecessary
  // allocations. TODO we could just flatten the whole thing into arrays in DrawData and store only indices in DrawLists
  if (draw_data.cmd_lists_count > draw_data.cmd_lists.size()) {
    draw_data.cmd_lists.resize(draw_data.cmd_lists_count);
  }

  for (auto i{0}; i < imgui_draw_data.CmdListsCount; i++) {
    auto const& draw_list{*imgui_draw_data.CmdLists[i]};
    auto& cmd_list{draw_data.cmd_lists[i]};

    cmd_list.cmd_buffer.assign(draw_list.CmdBuffer.begin(), draw_list.CmdBuffer.end());
    cmd_list.idx_buffer.assign(draw_list.IdxBuffer.begin(), draw_list.IdxBuffer.end());
    cmd_list.vtx_buffer.assign(draw_list.VtxBuffer.begin(), draw_list.VtxBuffer.end());
    cmd_list.flags = draw_list.Flags;
  }

  draw_data.textures.clear();

  for (auto const& ref : tex_refs_->GetReferences()) {
    if (auto const tex = std::get_if<ObjectPtr<Texture2D>>(&ref)) {
      auto const observed = tex->Get();
      draw_data.textures.emplace_back(observed ? tex_resolver_->Resolve(*observed, frame) : nullptr);
    } else if (auto const cubemap = std::get_if<ObjectPtr<Cubemap>>(&ref)) {
      auto const observed = cubemap->Get();
      draw_data.textures.emplace_back(observed ? tex_resolver_->Resolve(*observed, frame) : nullptr);
    } else {
      draw_data.textures.emplace_back(std::get<std::shared_ptr<rendering::RenderTarget>>(ref)->GetColorTex());
    }
  }

  if (fonts_dirty_) {
    unsigned char* fonts_tex_pixel_data;
    int fonts_tex_width;
    int fonts_tex_height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fonts_tex_pixel_data, &fonts_tex_width, &fonts_tex_height);

    font_tex_ = device_->CreateTexture(wand::TextureDesc{
      wand::TextureDimension::k2D, static_cast<UINT>(fonts_tex_width), static_cast<UINT>(fonts_tex_height), 1, 1,
      DXGI_FORMAT_R8G8B8A8_UNORM, 1, false, false, true, false
    }, wand::CpuAccess::kNone, nullptr);
    font_tex_->SetDebugName(L"UI Font Texture");

    frame.UploadTexture(font_tex_, 0, std::array{
      D3D12_SUBRESOURCE_DATA{
        fonts_tex_pixel_data, static_cast<LONG_PTR>(fonts_tex_width) * 4,
        static_cast<LONG_PTR>(fonts_tex_height) * static_cast<LONG_PTR>(fonts_tex_width) * 4
      }
    });

    fonts_dirty_ = false;
  }

  draw_data.font_tex = font_tex_;
}


auto ImGuiRenderer::RecordFrame(rendering::RenderFrame& frame) -> void {
  auto const frame_idx{frame.GetIndex()};

  auto const draw_data{&draw_data_[frame_idx]};

  // Avoid rendering when minimized
  if (draw_data->display_size.x <= 0.0f || draw_data->display_size.y <= 0.0f) {
    return;
  }

  auto const proj_mtx{
    Matrix4::OrthographicOffCenter(draw_data->display_pos.x, draw_data->display_pos.x + draw_data->display_size.x,
      draw_data->display_pos.y, draw_data->display_pos.y + draw_data->display_size.y, -1, 1)
  };

  auto& vb{vtx_buffers_[frame_idx]};
  auto& vb_ptr{(vb_ptrs_[frame_idx])};

  if (auto const vtx_data_byte_size{draw_data->total_vtx_count * sizeof(ImDrawVert)};
    !vb || vb->GetDesc().size < vtx_data_byte_size) {
    vb = CreateBufferWithView(*device_, wand::BufferDesc{
      .size = vtx_data_byte_size,
      .usage = wand::BufferUsage::kShaderResource
    }, wand::BufferViewDesc{
      .offset = 0,
      .size = vtx_data_byte_size,
      .stride = static_cast<UINT>(sizeof(ImDrawVert)),
      .usage = wand::BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kWrite);
    vb->GetBuffer()->SetDebugName(L"UI Vertex Buffer");
    vb_ptr = vb->GetBuffer()->Map();
  }

  auto& ib{idx_buffers_[frame_idx]};
  auto& ib_ptr{ib_ptrs_[frame_idx]};

  if (auto const idx_data_byte_size{draw_data->total_idx_count * sizeof(ImDrawIdx)};
    !ib || ib->GetDesc().size < idx_data_byte_size) {
    ib = device_->CreateBuffer(wand::BufferDesc{
      .size = idx_data_byte_size,
      .usage = wand::BufferUsage::kIndexBuffer
    }, wand::CpuAccess::kWrite);
    ib->SetDebugName(L"UI Index Buffer");
    ib_ptr = ib->Map();
  }

  auto constexpr idx_format{
    [] {
      if constexpr (sizeof(ImDrawIdx) == 2) {
        return DXGI_FORMAT_R16_UINT;
      } else {
        return DXGI_FORMAT_R32_UINT;
      }
    }()
  };

  auto vtx_dst{static_cast<ImDrawVert*>(vb_ptr)};
  auto idx_dst{static_cast<ImDrawIdx*>(ib_ptr)};

  for (auto i{0}; i < draw_data->cmd_lists_count; i++) {
    auto const imgui_cmd{draw_data->cmd_lists[i]};
    std::memcpy(vtx_dst, imgui_cmd.vtx_buffer.data(), imgui_cmd.vtx_buffer.size() * sizeof(ImDrawVert));
    std::memcpy(idx_dst, imgui_cmd.idx_buffer.data(), imgui_cmd.idx_buffer.size() * sizeof(ImDrawIdx));
    vtx_dst += imgui_cmd.vtx_buffer.size();
    idx_dst += imgui_cmd.idx_buffer.size();
  }

  auto const& rt{swap_chain_->GetCurrentTexture()};

  auto& cmd{frame.AcquireCommandList()};
  cmd.Begin(pso_.get());

  cmd.SetBlendFactor(std::array{0.f, 0.f, 0.f, 0.f});
  cmd.SetIndexBuffer(*ib, idx_format);
  cmd.SetPipelineParameters(PIPELINE_PARAM_INDEX(ImGuiDrawParams, proj_mtx),
    std::span{std::bit_cast<UINT const*>(&proj_mtx), 16});
  cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(ImGuiDrawParams, samp_idx), samp_.Get());
  cmd.SetShaderResource(PIPELINE_PARAM_INDEX(ImGuiDrawParams, vb_idx), *vb);
  cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  cmd.SetRenderTargets(std::span{std::array{(&rt)}.data(), 1}, nullptr);
  cmd.SetViewports(std::array<D3D12_VIEWPORT, 1>{
    CD3DX12_VIEWPORT{0.0f, 0.0f, draw_data->display_size.x, draw_data->display_size.y}
  });

  cmd.ClearRenderTarget(rt, std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own offset into them)
  auto global_vtx_offset{0};
  auto global_idx_offset{0};

  auto const& clip_off{draw_data->display_pos};
  for (auto i{0}; i < draw_data->cmd_lists_count; i++) {
    auto const& imgui_cmd{draw_data->cmd_lists[i]};

    for (auto j{0}; j < imgui_cmd.cmd_buffer.size(); j++) {
      auto const& draw_cmd{imgui_cmd.cmd_buffer[j]};

      if (draw_cmd.UserCallback != nullptr) {
        // TODO honor user callback
        // User callback, registered via ImDrawList::AddCallback()
        // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
        /*if (draw_cmd.UserCallback == ImDrawCallback_ResetRenderState) {
          // TODO reset render state - ImGui_ImplDX12_SetupRenderState(draw_data, ctx, fr);
        } else {
          draw_cmd.UserCallback(imgui_cmd, &draw_cmd);
        }*/
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        Vector2 const clip_min{draw_cmd.ClipRect.x - clip_off.x, draw_cmd.ClipRect.y - clip_off.y};
        Vector2 const clip_max{draw_cmd.ClipRect.z - clip_off.x, draw_cmd.ClipRect.w - clip_off.y};
        if (clip_max[0] <= clip_min[0] || clip_max[1] <= clip_min[1]) {
          continue;
        }

        cmd.SetScissorRects(std::array{
          D3D12_RECT{
            static_cast<LONG>(clip_min[0]), static_cast<LONG>(clip_min[1]), static_cast<LONG>(clip_max[0]),
            static_cast<LONG>(clip_max[1])
          }
        });

        if (auto const tex = ResolveImGuiTexture(draw_cmd.GetTexID(), frame_idx)) {
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(ImGuiDrawParams, tex_idx), *tex);
        } else {
          cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(ImGuiDrawParams, tex_idx), INVALID_RES_IDX);
        }

        cmd.DrawIndexedInstanced(draw_cmd.ElemCount, 1, draw_cmd.IdxOffset + global_idx_offset,
          draw_cmd.VtxOffset + global_vtx_offset, 0);
      }
    }
    global_idx_offset += static_cast<int>(imgui_cmd.idx_buffer.size());
    global_vtx_offset += static_cast<int>(imgui_cmd.vtx_buffer.size());
  }

  cmd.End();
  frame.EnqueueCommandList(cmd);
}


auto ImGuiRenderer::ResolveImGuiTexture(
  ImTextureID const id,
  std::uint32_t const frame_idx
) const -> ObserverPtr<wand::Texture> {
  auto const& draw_data = draw_data_[frame_idx];
  assert(draw_data.textures.size() <= kFontTexId && "ImGui texture reference list overlaps font tex ID!");

  if (id == kFontTexId) {
    return MakeObserver(draw_data.font_tex.get());
  }

  if (id >= draw_data.textures.size()) {
    return nullptr;
  }

  return MakeObserver(draw_data.textures[id].get());
}


ImTextureID const ImGuiRenderer::kFontTexId{std::numeric_limits<ImTextureID>::max()};
}
