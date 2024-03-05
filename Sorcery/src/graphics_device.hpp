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
class GraphicsDevice;

class Buffer;
class Texture;
class PipelineState;
class CommandList;
class SwapChain;

template<typename T>
class DeviceChildDeleter;

using UniqueBufferHandle = std::unique_ptr<Buffer, DeviceChildDeleter<Buffer>>;
using UniqueTextureHandle = std::unique_ptr<Texture, DeviceChildDeleter<Texture>>;
using UniquePipelineStateHandle = std::unique_ptr<PipelineState, DeviceChildDeleter<PipelineState>>;
using UniqueCommandListHandle = std::unique_ptr<CommandList, DeviceChildDeleter<CommandList>>;
using UniqueSwapChainHandle = std::unique_ptr<SwapChain, DeviceChildDeleter<SwapChain>>;
class UniqueSamplerHandle;


struct BufferDesc {
  UINT size;
  UINT stride;
  bool constant_buffer;
  bool shader_resource;
  bool unordered_access;
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

  bool depth_stencil;
  bool render_target;
  bool shader_resource;
  bool unordered_access;
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


class DescriptorHeap {
public:
  [[nodiscard]] auto Allocate() -> UINT;
  auto Release(UINT index) -> void;

  [[nodiscard]] auto GetDescriptorCpuHandle(UINT descriptor_index) const -> D3D12_CPU_DESCRIPTOR_HANDLE;
  [[nodiscard]] auto GetDescriptorGpuHandle(UINT descriptor_index) const -> D3D12_GPU_DESCRIPTOR_HANDLE;

  [[nodiscard]] auto GetInternalPtr() const -> ID3D12DescriptorHeap*;

  DescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, ID3D12Device& device);
  DescriptorHeap(DescriptorHeap const&) = delete;
  DescriptorHeap(DescriptorHeap&&) = delete;

  ~DescriptorHeap() = default;

  auto operator=(DescriptorHeap const&) -> void = delete;
  auto operator=(DescriptorHeap&&) -> void = delete;

private:
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
  std::vector<UINT> free_indices_;
  std::mutex mutex_;
  UINT increment_size_;
};


class RootSignatureCache {
public:
  auto Add(std::uint8_t num_params, Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature) -> void;
  [[nodiscard]] auto Get(std::uint8_t num_params) -> Microsoft::WRL::ComPtr<ID3D12RootSignature>;

private:
  std::unordered_map<std::uint8_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>> root_signatures_;
  std::mutex mutex_;
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
  [[nodiscard]] auto CreateCommandList() -> UniqueCommandListHandle;
  [[nodiscard]] auto CreateFence(UINT64 initial_value) -> Microsoft::WRL::ComPtr<ID3D12Fence1>;
  [[nodiscard]] auto CreateSwapChain(SwapChainDesc const& desc, HWND window_handle) -> UniqueSwapChainHandle;
  [[nodiscard]] auto CreateSampler(D3D12_SAMPLER_DESC const& desc) -> UniqueSamplerHandle;

  auto DestroyBuffer(Buffer const* buffer) -> void;
  auto DestroyTexture(Texture const* texture) -> void;
  auto DestroyPipelineState(PipelineState const* pipeline_state) -> void;
  auto DestroyCommandList(CommandList const* command_list) -> void;
  auto DestroySwapChain(SwapChain const* swap_chain) -> void;
  auto DestroySampler(UINT sampler) -> void;

  [[nodiscard]] auto WaitFence(ID3D12Fence& fence, UINT64 wait_value) const -> bool;
  [[nodiscard]] auto SignalFence(ID3D12Fence& fence, UINT64 signal_value) const -> bool;
  auto ExecuteCommandLists(std::span<CommandList const> cmd_lists) const -> void;

  [[nodiscard]] auto SwapChainGetBuffers(SwapChain const& swap_chain) const -> std::span<UniqueTextureHandle const>;
  [[nodiscard]] auto SwapChainGetCurrentBufferIndex(SwapChain const& swap_chain) const -> UINT;
  [[nodiscard]] auto SwapChainPresent(SwapChain const& swap_chain, UINT sync_interval) const -> bool;
  [[nodiscard]] auto SwapChainResize(SwapChain& swap_chain, UINT width, UINT height) -> bool;

private:
  GraphicsDevice(Microsoft::WRL::ComPtr<IDXGIFactory7> factory, Microsoft::WRL::ComPtr<ID3D12Device10> device,
                 Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> res_desc_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sampler_heap,
                 Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue);

  auto SwapChainCreateTextures(SwapChain& swap_chain) -> bool;

  static UINT const rtv_heap_size_;
  static UINT const dsv_heap_size_;
  static UINT const res_desc_heap_size_;
  static UINT const sampler_heap_size_;

  Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
  Microsoft::WRL::ComPtr<ID3D12Device10> device_;
  Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator_;

  DescriptorHeap rtv_heap_;
  DescriptorHeap dsv_heap_;
  DescriptorHeap res_desc_heap_;
  DescriptorHeap sampler_heap_;

  Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;

  RootSignatureCache root_signatures_;

  UINT swap_chain_flags_{0};
  UINT present_flags_{0};
};


class Buffer {
public:
  [[nodiscard]] auto GetConstantBuffer() const -> UINT;
  [[nodiscard]] auto GetShaderResource() const -> UINT;
  [[nodiscard]] auto GetUnorderedAccess() const -> UINT;

private:
  Buffer(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
         UINT cbv, UINT srv, UINT uav);

  Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation_;
  Microsoft::WRL::ComPtr<ID3D12Resource2> resource_;
  UINT cbv_;
  UINT srv_;
  UINT uav_;

  friend GraphicsDevice;
  friend CommandList;
};


class Texture {
public:
  [[nodiscard]] auto GetShaderResource() const -> UINT;
  [[nodiscard]] auto GetUnorderedAccess() const -> UINT;

private:
  Texture(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
          UINT dsv, UINT rtv, UINT srv, UINT uav);

  Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation_;
  Microsoft::WRL::ComPtr<ID3D12Resource2> resource_;
  UINT dsv_;
  UINT rtv_;
  UINT srv_;
  UINT uav_;

  friend GraphicsDevice;
  friend CommandList;
};


class PipelineState {
  PipelineState(Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature,
                Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state, std::uint8_t num_params, bool is_compute);

  Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_;
  std::uint8_t num_params_;
  bool is_compute_;

  friend GraphicsDevice;
  friend CommandList;
};


class CommandList {
public:
  [[nodiscard]] auto Begin(PipelineState const& pipeline_state) -> bool;
  [[nodiscard]] auto End() const -> bool;
  auto Barrier(std::span<GlobalBarrier const> global_barriers, std::span<BufferBarrier const> buffer_barriers,
               std::span<TextureBarrier const> texture_barriers) const -> void;
  auto ClearDepthStencil(Texture const& tex, D3D12_CLEAR_FLAGS clear_flags, FLOAT depth, UINT8 stencil,
                         std::span<D3D12_RECT const> rects) const -> void;
  auto ClearRenderTarget(Texture const& tex, std::span<FLOAT const, 4> color_rgba,
                         std::span<D3D12_RECT const> rects) const -> void;
  auto CopyBuffer(Buffer const& dst, Buffer const& src) const -> void;
  auto CopyBufferRegion(Buffer const& dst, UINT64 dst_offset, Buffer const& src, UINT64 src_offset,
                        UINT64 num_bytes) const -> void;
  auto CopyTexture(Texture const& dst, Texture const& src) const -> void;
  auto CopyTextureRegion(Texture const& dst, UINT dst_subresource_index, UINT dst_x, UINT dst_y, UINT dst_z,
                         Texture const& src, UINT src_subresource_index, D3D12_BOX const* src_box) const -> void;
  auto CopyTextureRegion(Texture const& dst, UINT dst_subresource_index, UINT dst_x, UINT dst_y, UINT dst_z,
                         Buffer const& src, D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& src_footprint) const -> void;
  auto Dispatch(UINT thread_group_count_x, UINT thread_group_count_y, UINT thread_group_count_z) const -> void;
  auto DispatchMesh(UINT thread_group_count_x, UINT thread_group_count_y, UINT thread_group_count_z) const -> void;
  auto DrawIndexedInstanced(UINT index_count_per_instance, UINT instance_count, UINT start_index_location,
                            INT base_vertex_location, UINT start_instance_location) const -> void;
  auto DrawInstanced(UINT vertex_count_per_instance, UINT instance_count, UINT start_vertex_location,
                     UINT start_instance_location) const -> void;
  auto SetBlendFactor(std::span<FLOAT const, 4> blend_factor) const -> void;
  auto SetRenderTargets(std::span<Texture const> render_targets, Texture const* depth_stencil) const -> void;
  auto SetStencilRef(UINT stencil_ref) const -> void;
  auto SetScissorRects(std::span<D3D12_RECT const> rects) const -> void;
  auto SetViewports(std::span<D3D12_VIEWPORT const> viewports) const -> void;
  auto SetPipelineParameter(UINT index, UINT value) const -> void;
  auto SetPipelineParameters(UINT index, std::span<UINT const> values) const -> void;
  auto SetPipelineState(PipelineState const& pipeline_state) -> void;
  auto SetStreamOutputTargets(UINT start_slot, std::span<D3D12_STREAM_OUTPUT_BUFFER_VIEW const> views) const -> void;

private:
  auto SetRootSignature(std::uint8_t num_params) const -> void;

  CommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator,
              Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmd_list, DescriptorHeap const* dsv_heap,
              DescriptorHeap const* rtv_heap, DescriptorHeap const* res_desc_heap, DescriptorHeap const* sampler_heap,
              RootSignatureCache* root_signatures);

  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator_;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmd_list_;
  DescriptorHeap const* dsv_heap_;
  DescriptorHeap const* rtv_heap_;
  DescriptorHeap const* res_desc_heap_;
  DescriptorHeap const* sampler_heap_;
  RootSignatureCache* root_signatures_;
  bool compute_pipeline_set_{false};

  friend GraphicsDevice;
};


class SwapChain {
  explicit SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain);

  Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_;
  std::vector<UniqueTextureHandle> textures_;

  friend GraphicsDevice;
};


template<typename T>
class DeviceChildDeleter {
public:
  explicit DeviceChildDeleter(GraphicsDevice& device) :
    device_{&device} {}


  auto operator()(T const* device_child) const -> void;

private:
  GraphicsDevice* device_;
};


auto DeviceChildDeleter<Buffer>::operator()(Buffer const* device_child) const -> void;
auto DeviceChildDeleter<Texture>::operator()(Texture const* device_child) const -> void;
auto DeviceChildDeleter<PipelineState>::operator()(PipelineState const* device_child) const -> void;
auto DeviceChildDeleter<CommandList>::operator()(CommandList const* device_child) const -> void;
auto DeviceChildDeleter<SwapChain>::operator()(SwapChain const* device_child) const -> void;


class UniqueSamplerHandle {
public:
  [[nodiscard]] auto Get() const -> UINT;

  UniqueSamplerHandle(UINT sampler, GraphicsDevice& device);
  UniqueSamplerHandle(UniqueSamplerHandle const&) = delete;
  UniqueSamplerHandle(UniqueSamplerHandle&& other) noexcept;

  ~UniqueSamplerHandle();

  auto operator=(UniqueSamplerHandle const&) -> void = delete;
  auto operator=(UniqueSamplerHandle&& other) noexcept -> UniqueSamplerHandle&;

private:
  GraphicsDevice* device_;
  UINT sampler_;
};
}
