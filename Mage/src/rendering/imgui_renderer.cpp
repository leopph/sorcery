// Based on Dear ImGui's DX12 renderer backend
// Adapted to Sorcery's GraphicsDevice

#include "imgui_renderer.hpp"
#include "imgui_shader_interop.h"

#ifndef NDEBUG
#include "generated/Debug/imgui_ps.h"
#include "generated/Debug/imgui_vs.h"
#else
#include "generated/Release/imgui_ps.h"
#include "generated/Release/imgui_vs.h"
#endif

#include <imgui.h>

#include <bit>
#include <limits>
#include <stdexcept>


// DirectX data
struct ImGui_ImplDX12_RenderBuffers;


struct ImGui_ImplDX12_Data {
  ID3D12Device* pd3dDevice;
  ID3D12RootSignature* pRootSignature;
  ID3D12PipelineState* pPipelineState;
  DXGI_FORMAT RTVFormat;
  ID3D12Resource* pFontTextureResource;
  D3D12_CPU_DESCRIPTOR_HANDLE hFontSrvCpuDescHandle;
  D3D12_GPU_DESCRIPTOR_HANDLE hFontSrvGpuDescHandle;
  ID3D12DescriptorHeap* pd3dSrvDescHeap;
  UINT numFramesInFlight;

  ImGui_ImplDX12_RenderBuffers* pFrameResources;
  UINT frameIndex;


  ImGui_ImplDX12_Data() {
    memset((void*)this, 0, sizeof(*this));
    frameIndex = UINT_MAX;
  }
};


// Buffers used during the rendering of a frame
struct ImGui_ImplDX12_RenderBuffers {
  ID3D12Resource* IndexBuffer;
  ID3D12Resource* VertexBuffer;
  int IndexBufferSize;
  int VertexBufferSize;
};


struct VERTEX_CONSTANT_BUFFER_DX12 {
  float mvp[4][4];
};


namespace sorcery::mage {
ImGuiRenderer::ImGuiRenderer(graphics::GraphicsDevice& device, Window const& window,
                             rendering::RenderManager& render_manager) :
  device_{&device},
  window_{&window},
  render_manager_{&render_manager} {
  pso_ = device_->CreatePipelineState(graphics::PipelineDesc{
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

  unsigned char* fonts_tex_pixel_data;
  int fonts_tex_width;
  int fonts_tex_height;
  ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fonts_tex_pixel_data, &fonts_tex_width, &fonts_tex_height);

  D3D12_SUBRESOURCE_DATA const fonts_upload_data{
    fonts_tex_pixel_data, static_cast<LONG_PTR>(fonts_tex_width) * 4,
    static_cast<LONG_PTR>(fonts_tex_height) * static_cast<LONG_PTR>(fonts_tex_width) * 4
  };

  fonts_tex_ = device_->CreateTexture(graphics::TextureDesc{
    graphics::TextureDimension::k2D, static_cast<UINT>(fonts_tex_width), static_cast<UINT>(fonts_tex_height), 1, 1,
    DXGI_FORMAT_R8G8B8A8_UNORM, {1, 0}, D3D12_RESOURCE_FLAG_NONE, false, false, true, false
  }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr);

  auto const fonts_upload_buf{
    device_->CreateBuffer(graphics::BufferDesc{
      static_cast<UINT>(fonts_tex_->GetRequiredIntermediateSize()), 0, false, false, false
    }, D3D12_HEAP_TYPE_UPLOAD)
  };

  auto& cmd{render_manager_->AcquireCommandList()};

  if (!cmd.Begin(nullptr)) {
    throw std::runtime_error{"Failed to begin command list for font texture upload."};
  }

  std::ignore = cmd.UpdateSubresources(*fonts_tex_, *fonts_upload_buf, 0, 0, 1, &fonts_upload_data);

  cmd.Barrier({}, {}, std::array{
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_NO_ACCESS,
      D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE, fonts_tex_.get(), {0, 1, 0, 1, 0, 1},
      D3D12_TEXTURE_BARRIER_FLAG_DISCARD
    }
  });

  if (!cmd.End()) {
    throw std::runtime_error{"Failed to end command list for font texture upload."};
  }

  device_->ExecuteCommandLists(std::span{&cmd, 1});

  if (!device_->WaitIdle()) {
    throw std::runtime_error{"Failed to wait for font texture upload."};
  }

  ImGui::GetIO().Fonts->SetTexID(fonts_tex_.get());
}


auto ImGuiRenderer::Render(ImDrawData* draw_data) -> void {
  // Avoid rendering when minimized
  if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) {
    return;
  }

  auto const frame_idx{render_manager_->GetCurrentFrameIndex()};

  auto const proj_mtx{
    Matrix4::OrthographicOffCenter(draw_data->DisplayPos.x, draw_data->DisplayPos.x + draw_data->DisplaySize.x,
      draw_data->DisplayPos.y, draw_data->DisplayPos.y + draw_data->DisplaySize.y, -1, 1)
  };

  auto& vb{vtx_buffers_[frame_idx]};
  auto& vb_ptr{(vb_ptrs_[frame_idx])};

  if (auto const vtx_data_byte_size{draw_data->TotalVtxCount * sizeof(ImDrawVert)};
    !vb || vb->GetDesc().Width < vtx_data_byte_size) {
    vb = device_->CreateBuffer(graphics::BufferDesc{
      vtx_data_byte_size, static_cast<UINT>(sizeof(ImDrawVert)), false, true, false
    }, D3D12_HEAP_TYPE_UPLOAD);
    vb_ptr = vb->Map();
  }

  auto& ib{idx_buffers_[frame_idx]};
  auto& ib_ptr{ib_ptrs_[frame_idx]};

  if (auto const idx_data_byte_size{draw_data->TotalIdxCount * sizeof(ImDrawIdx)};
    !ib || ib->GetDesc().Width < idx_data_byte_size) {
    ib = device_->CreateBuffer(graphics::BufferDesc{idx_data_byte_size, 0, false, false, false},
      D3D12_HEAP_TYPE_UPLOAD);
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

  for (auto i{0}; i < draw_data->CmdListsCount; i++) {
    auto const imgui_cmd{draw_data->CmdLists[i]};
    std::memcpy(vtx_dst, imgui_cmd->VtxBuffer.Data, imgui_cmd->VtxBuffer.Size * sizeof(ImDrawVert));
    std::memcpy(idx_dst, imgui_cmd->IdxBuffer.Data, imgui_cmd->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtx_dst += imgui_cmd->VtxBuffer.Size;
    idx_dst += imgui_cmd->IdxBuffer.Size;
  }

  auto& cmd{render_manager_->AcquireCommandList()};

  if (!cmd.Begin(pso_.get())) {
    throw std::runtime_error{"Failed to begin command buffer for ImGui rendering."};
  }

  auto const& rt{
    *device_->SwapChainGetBuffers(*window_->GetSwapChain())[device_->SwapChainGetCurrentBufferIndex(
      *window_->GetSwapChain())]
  };

  cmd.Barrier({}, {}, std::array{
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_SYNC_RENDER_TARGET, D3D12_BARRIER_ACCESS_NO_ACCESS,
      D3D12_BARRIER_ACCESS_RENDER_TARGET, D3D12_BARRIER_LAYOUT_UNDEFINED, D3D12_BARRIER_LAYOUT_RENDER_TARGET, &rt,
      D3D12_BARRIER_SUBRESOURCE_RANGE{0, 1, 0, 1, 0, 1}, D3D12_TEXTURE_BARRIER_FLAG_NONE
    }
  });
  cmd.SetBlendFactor(std::array{0.f, 0.f, 0.f, 0.f});
  cmd.SetIndexBuffer(*ib, idx_format);
  cmd.SetPipelineParameters(offsetof(ImGuiDrawParams, proj_mtx) / 4,
    std::span{std::bit_cast<UINT const*>(&proj_mtx), 16});
  cmd.SetPipelineParameter(offsetof(ImGuiDrawParams, samp_idx) / 4, samp_.Get());
  cmd.SetPipelineParameter(offsetof(ImGuiDrawParams, vb_idx) / 4, vb->GetShaderResource());
  cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  cmd.SetRenderTargets(std::span{&rt, 1}, nullptr);
  cmd.SetViewports(std::array<D3D12_VIEWPORT, 1>{
    CD3DX12_VIEWPORT{0.0f, 0.0f, draw_data->DisplaySize.x, draw_data->DisplaySize.y}
  });

  cmd.ClearRenderTarget(rt, std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own offset into them)
  auto global_vtx_offset{0};
  auto global_idx_offset{0};

  auto const& clip_off{draw_data->DisplayPos};
  for (auto i{0}; i < draw_data->CmdListsCount; i++) {
    ImDrawList const* imgui_cmd{draw_data->CmdLists[i]};

    for (auto j{0}; j < imgui_cmd->CmdBuffer.Size; j++) {
      auto const& draw_cmd{imgui_cmd->CmdBuffer[j]};

      if (draw_cmd.UserCallback != nullptr) {
        // User callback, registered via ImDrawList::AddCallback()
        // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
        if (draw_cmd.UserCallback == ImDrawCallback_ResetRenderState) {
          // TODO reset render state - ImGui_ImplDX12_SetupRenderState(draw_data, ctx, fr);
        } else {
          draw_cmd.UserCallback(imgui_cmd, &draw_cmd);
        }
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

        cmd.SetPipelineParameter(offsetof(ImGuiDrawParams, tex_idx) / 4,
          static_cast<graphics::Texture*>(draw_cmd.GetTexID())->GetShaderResource());

        cmd.DrawIndexedInstanced(draw_cmd.ElemCount, 1, draw_cmd.IdxOffset + global_idx_offset,
          draw_cmd.VtxOffset + global_vtx_offset, 0);
      }
    }
    global_idx_offset += imgui_cmd->IdxBuffer.Size;
    global_vtx_offset += imgui_cmd->VtxBuffer.Size;
  }

  cmd.Barrier({}, {}, std::array{
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_RENDER_TARGET, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_RENDER_TARGET,
      D3D12_BARRIER_ACCESS_NO_ACCESS, D3D12_BARRIER_LAYOUT_RENDER_TARGET, D3D12_BARRIER_LAYOUT_PRESENT, &rt,
      D3D12_BARRIER_SUBRESOURCE_RANGE{0, 1, 0, 1, 0, 1}, D3D12_TEXTURE_BARRIER_FLAG_NONE
    }
  });

  if (!cmd.End()) {
    throw std::runtime_error{"Failed to end command buffer for ImGui rendering."};
  }

  device_->ExecuteCommandLists(std::span{&cmd, 1});
}
}
