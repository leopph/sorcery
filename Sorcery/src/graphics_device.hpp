#pragma once

#include "graphics_platform.hpp"

#include <D3D12MemAlloc.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <unordered_map>
#include <vector>


extern "C" {
__declspec(dllexport) extern UINT const D3D12SDKVersion;
__declspec(dllexport) extern char const* D3D12SDKPath;
}


namespace sorcery::graphics {
class Buffer;
class Texture;
struct PipelineState;
struct CommandList;


struct SwapChain {
  Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain;
  std::vector<Texture> textures;
};


struct BufferDeleter {
  auto operator()(Buffer const* buffer) const -> void;
};


struct TextureDeleter {
  auto operator()(Texture const* texture) const -> void;
};


struct PipelineStateDeleter {
  auto operator()(PipelineState const* pipeline_state) const -> void;
};


struct CommandListDeleter {
  auto operator()(CommandList const* cmd_list) const -> void;
};


struct SwapChainDeleter {
  auto operator()(SwapChain const* swap_chain) const -> void;
};


using UniqueBufferHandle = std::unique_ptr<Buffer, BufferDeleter>;
using UniqueTextureHandle = std::unique_ptr<Texture, TextureDeleter>;
using UniquePipelineStateHandle = std::unique_ptr<PipelineState, PipelineStateDeleter>;
using UniqueCommandListHandle = std::unique_ptr<CommandList, CommandListDeleter>;
using UniqueSwapChainHandle = std::unique_ptr<SwapChain, SwapChainDeleter>;


struct BufferDesc {
  UINT size;
  UINT stride;
  bool cbv;
  bool srv;
  bool uav;
};


enum class TextureDimension {
  k1D,
  k2D,
  k3D,
  kCube
};


struct TextureDesc {
  TextureDimension dimension;
  UINT width;
  UINT height;
  UINT16 depth_or_array_size;
  UINT16 mip_levels;
  DXGI_FORMAT format;
  DXGI_SAMPLE_DESC sample_desc;
  D3D12_RESOURCE_FLAGS flags;

  bool dsv;
  bool rtv;
  bool srv;
  bool uav;
};


struct PipelineStateDesc {
  CD3DX12_PIPELINE_STATE_STREAM_VS vs;
  CD3DX12_PIPELINE_STATE_STREAM_GS gs;
  CD3DX12_PIPELINE_STATE_STREAM_STREAM_OUTPUT stream_output;
  CD3DX12_PIPELINE_STATE_STREAM_HS hs;
  CD3DX12_PIPELINE_STATE_STREAM_DS ds;
  CD3DX12_PIPELINE_STATE_STREAM_PS ps;
  CD3DX12_PIPELINE_STATE_STREAM_AS as;
  CD3DX12_PIPELINE_STATE_STREAM_MS ms;
  CD3DX12_PIPELINE_STATE_STREAM_CS cs;
  CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend_state;
  CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1 depth_stencil_state;
  CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT dsv_format;
  CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer_state;
  CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtv_formats;
  CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC sample_desc;
  CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK sample_mask;
  CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING view_instancing;
};


struct SwapChainDesc {
  UINT width;
  UINT height;
  UINT buffer_count;
  DXGI_USAGE usage;
  DXGI_SCALING scaling;
};


struct GlobalBarrier {
  D3D12_BARRIER_SYNC sync_before;
  D3D12_BARRIER_SYNC sync_after;
  D3D12_BARRIER_ACCESS access_before;
  D3D12_BARRIER_ACCESS access_after;
};


struct TextureBarrier {
  D3D12_BARRIER_SYNC sync_before;
  D3D12_BARRIER_SYNC sync_after;
  D3D12_BARRIER_ACCESS access_before;
  D3D12_BARRIER_ACCESS access_after;
  D3D12_BARRIER_LAYOUT layout_before;
  D3D12_BARRIER_LAYOUT layout_after;
  Texture const* texture;
  D3D12_BARRIER_SUBRESOURCE_RANGE subresources;
  D3D12_TEXTURE_BARRIER_FLAGS flags;
};


struct BufferBarrier {
  D3D12_BARRIER_SYNC sync_before;
  D3D12_BARRIER_SYNC sync_after;
  D3D12_BARRIER_ACCESS access_before;
  D3D12_BARRIER_ACCESS access_after;
  Buffer const* buffer;
  UINT64 offset;
  UINT64 size;
};


class GraphicsDevice {
public:
  [[nodiscard]] static auto New(bool enable_debug) -> std::unique_ptr<GraphicsDevice>;

  [[nodiscard]] auto CreateBuffer(BufferDesc const& desc, D3D12_HEAP_TYPE heap_type) -> UniqueBufferHandle;
  [[nodiscard]] auto CreateTexture(TextureDesc const& desc, D3D12_HEAP_TYPE heap_type,
                                   D3D12_BARRIER_LAYOUT initial_layout,
                                   D3D12_CLEAR_VALUE const* clear_value) -> UniqueTextureHandle;
  [[nodiscard]] auto CreatePipelineState(PipelineStateDesc const& desc,
                                         std::uint8_t num_32_bit_params) -> UniquePipelineStateHandle;
  [[nodiscard]] auto CreateCommandList() const -> UniqueCommandListHandle;
  [[nodiscard]] auto CreateFence(UINT64 initial_value) const -> Microsoft::WRL::ComPtr<ID3D12Fence1>;
  [[nodiscard]] auto CreateSwapChain(SwapChainDesc const& desc, HWND window_handle) -> UniqueSwapChainHandle;

  [[nodiscard]] auto WaitFence(ID3D12Fence& fence, UINT64 wait_value) const -> bool;
  [[nodiscard]] auto SignalFence(ID3D12Fence& fence, UINT64 signal_value) const -> bool;
  auto ExecuteCommandLists(std::span<CommandList const> cmd_lists) -> void;

  [[nodiscard]] auto CmdBegin(CommandList& cmd_list, PipelineState const& pipeline_state) const -> bool;
  [[nodiscard]] auto CmdEnd(CommandList const& cmd_list) const -> bool;
  auto CmdBarrier(CommandList const& cmd_list, std::span<GlobalBarrier const> global_barriers,
                  std::span<BufferBarrier const> buffer_barriers,
                  std::span<TextureBarrier const> texture_barriers) const -> void;
  auto CmdClearDepthStencil(CommandList const& cmd_list, Texture const& tex, D3D12_CLEAR_FLAGS clear_flags, FLOAT depth,
                            UINT8 stencil, std::span<D3D12_RECT const> rects) const -> void;
  auto CmdClearRenderTarget(CommandList const& cmd_list, Texture const& tex, std::span<FLOAT const, 4> color_rgba,
                            std::span<D3D12_RECT const> rects) const -> void;
  auto CmdCopyBuffer(CommandList const& cmd_list, Buffer const& dst, Buffer const& src) const -> void;
  auto CmdCopyBufferRegion(CommandList const& cmd_list, Buffer const& dst, UINT64 dst_offset, Buffer const& src,
                           UINT64 src_offset, UINT64 num_bytes) const -> void;
  auto CmdCopyTexture(CommandList const& cmd_list, Texture const& dst, Texture const& src) -> void;
  auto CmdCopyTextureRegion(CommandList const& cmd_list, Texture const& dst, UINT dst_subresource_index, UINT dst_x,
                            UINT dst_y, UINT dst_z, Texture const& src, UINT src_subresource_index,
                            D3D12_BOX const* src_box) const -> void;
  auto CmdCopyTextureRegion(CommandList const& cmd_list, Texture const& dst, UINT dst_subresource_index, UINT dst_x,
                            UINT dst_y, UINT dst_z, Buffer const& src,
                            D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& src_footprint) const -> void;
  auto CmdDispatch(CommandList const& cmd_list, UINT thread_group_count_x, UINT thread_group_count_y,
                   UINT thread_group_count_z) const -> void;
  auto CmdDispatchMesh(CommandList const& cmd_list, UINT thread_group_count_x, UINT thread_group_count_y,
                       UINT thread_group_count_z) const -> void;
  auto CmdDrawIndexedInstanced(CommandList const& cmd_list, UINT index_count_per_instance, UINT instance_count,
                               UINT start_index_location, INT base_vertex_location,
                               UINT start_instance_location) const -> void;
  auto CmdDrawInstanced(CommandList const& cmd_list, UINT vertex_count_per_instance, UINT instance_count,
                        UINT start_vertex_location, UINT start_instance_location) const -> void;
  auto CmdSetBlendFactor(CommandList const& cmd_list, std::span<FLOAT const, 4> blend_factor) const -> void;
  auto CmdSetRenderTargets(CommandList const& cmd_list, std::span<Texture const> render_targets,
                           Texture const* depth_stencil) const -> void;
  auto CmdSetStencilRef(CommandList const& cmd_list, UINT stencil_ref) const -> void;
  auto CmdSetScissorRects(CommandList const& cmd_list, std::span<D3D12_RECT const> rects) const -> void;
  auto CmdSetViewports(CommandList const& cmd_list, std::span<D3D12_VIEWPORT const> viewports) const -> void;
  auto CmdSetPipelineParameter(CommandList const& cmd_list, UINT index, UINT value) const -> void;
  auto CmdSetPipelineParameters(CommandList const& cmd_list, UINT index, std::span<UINT const> values) const -> void;
  auto CmdSetPipelineState(CommandList& cmd_list, PipelineState const& pipeline_state) const -> void;
  auto CmdSetStreamOutputTargets(CommandList const& cmd_list, UINT start_slot,
                                 std::span<D3D12_STREAM_OUTPUT_BUFFER_VIEW const> views) const -> void;

  [[nodiscard]] auto SwapChainGetBuffers(SwapChain const& swap_chain) const -> std::span<Texture const>;
  [[nodiscard]] auto SwapChainGetCurrentBufferIndex(SwapChain const& swap_chain) const -> UINT;
  [[nodiscard]] auto SwapChainPresent(SwapChain const& swap_chain, UINT sync_interval) const -> bool;
  [[nodiscard]] auto SwapChainResize(SwapChain& swap_chain, UINT width, UINT height) -> bool;

private:
  friend class Buffer;
  friend class Texture;

  GraphicsDevice(Microsoft::WRL::ComPtr<IDXGIFactory7> factory, Microsoft::WRL::ComPtr<ID3D12Device10> device,
                 Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> res_desc_heap,
                 Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue);

  [[nodiscard]] auto AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE type) -> UINT;
  auto ReleaseDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT idx) -> void;

  auto SetRootSignature(CommandList const& cmd_list, std::uint8_t num_params) const -> void;

  auto SwapChainCreateTextures(SwapChain& swap_chain) -> bool;

  static UINT const rtv_heap_size_;
  static UINT const dsv_heap_size_;
  static UINT const res_desc_heap_size_;
  static UINT const invalid_resource_index_;

  Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
  Microsoft::WRL::ComPtr<ID3D12Device10> device_;
  Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator_;

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap_;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap_;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> res_desc_heap_;

  std::vector<std::uint32_t> rtv_free_indices_;
  std::vector<std::uint32_t> dsv_free_indices_;
  std::vector<std::uint32_t> res_desc_free_indices_;

  std::mutex rtv_indices_mutex_;
  std::mutex dsv_indices_mutex_;
  std::mutex res_desc_free_indices_mutex_;

  Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;

  D3D12_CPU_DESCRIPTOR_HANDLE rtv_heap_start_;
  D3D12_CPU_DESCRIPTOR_HANDLE dsv_heap_start_;
  D3D12_CPU_DESCRIPTOR_HANDLE res_desc_heap_start_;

  UINT rtv_heap_increment_;
  UINT dsv_heap_increment_;
  UINT res_desc_heap_increment_;

  std::mutex root_signature_mutex_;

  std::unordered_map<std::uint8_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>> root_signatures_;

  std::mutex cmd_list_submission_mutex_;
  std::vector<ID3D12CommandList*> cmd_list_submission_buffer_;

  UINT swap_chain_flags_{0};
  UINT present_flags_{0};
};
}
