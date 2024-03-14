#pragma once

#include "../Core.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <D3D12MemAlloc.h>

#include <atomic>
#include <concepts>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

// Returns the index of the member if all the pipeline parameters are considered a single buffer of the specified type.
#define PIPELINE_PARAM_INDEX(BufferType, MemberName) offsetof(BufferType, MemberName) / 4


namespace sorcery::graphics {
class GraphicsDevice;

class Buffer;
class Texture;
class PipelineState;
class CommandList;
class Fence;
class SwapChain;
using Sampler = UINT;


namespace details {
UINT constexpr kInvalidResourceIndex{static_cast<UINT>(-1)};

template<typename T>concept DeviceChild = std::same_as<T, Buffer> || std::same_as<T, Texture> || std::same_as<
                                            T, PipelineState> || std::same_as<T, CommandList> || std::same_as<T, Fence>
                                          || std::same_as<T, SwapChain>;

template<DeviceChild T>
class DeviceChildDeleter;
}


template<details::DeviceChild T>
using UniqueDeviceChildHandle = std::unique_ptr<T, details::DeviceChildDeleter<T>>;
template<details::DeviceChild T>
using SharedDeviceChildHandle = std::shared_ptr<T>;
class UniqueSamplerHandle;


struct BufferDesc {
  UINT64 size;
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


struct PipelineDesc {
  D3D12_PRIMITIVE_TOPOLOGY_TYPE primitive_topology_type{D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE};
  CD3DX12_SHADER_BYTECODE vs;
  CD3DX12_SHADER_BYTECODE gs;
  D3D12_STREAM_OUTPUT_DESC stream_output;
  CD3DX12_SHADER_BYTECODE hs;
  CD3DX12_SHADER_BYTECODE ds;
  CD3DX12_SHADER_BYTECODE ps;
  CD3DX12_SHADER_BYTECODE as;
  CD3DX12_SHADER_BYTECODE ms;
  CD3DX12_SHADER_BYTECODE cs;
  CD3DX12_BLEND_DESC blend_state{D3D12_DEFAULT};
  CD3DX12_DEPTH_STENCIL_DESC1 depth_stencil_state{D3D12_DEFAULT};
  DXGI_FORMAT ds_format;
  CD3DX12_RASTERIZER_DESC rasterizer_state{D3D12_DEFAULT};
  CD3DX12_RT_FORMAT_ARRAY rt_formats;
  DXGI_SAMPLE_DESC sample_desc{1, 0};
  UINT sample_mask{D3D12_DEFAULT_SAMPLE_MASK};
  CD3DX12_VIEW_INSTANCING_DESC view_instancing_desc{D3D12_DEFAULT};
};


struct SwapChainDesc {
  UINT width;
  UINT height;
  UINT buffer_count;
  DXGI_FORMAT format;
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


struct AliasedTextureCreateInfo {
  TextureDesc desc;
  D3D12_BARRIER_LAYOUT initial_layout;
  D3D12_CLEAR_VALUE* clear_value;
};


namespace details {
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
  UINT reserved_idx_count_{0};
};


class RootSignatureCache {
public:
  auto Add(std::uint8_t num_params, Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature) -> void;
  [[nodiscard]] auto Get(std::uint8_t num_params) -> Microsoft::WRL::ComPtr<ID3D12RootSignature>;

private:
  std::unordered_map<std::uint8_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>> root_signatures_;
  std::mutex mutex_;
};
}


class GraphicsDevice {
public:
  [[nodiscard]] LEOPPHAPI static auto New(bool enable_debug) -> std::unique_ptr<GraphicsDevice>;

  [[nodiscard]] LEOPPHAPI auto CreateBuffer(BufferDesc const& desc,
                                            D3D12_HEAP_TYPE heap_type) -> SharedDeviceChildHandle<Buffer>;
  [[nodiscard]] LEOPPHAPI auto CreateTexture(TextureDesc const& desc, D3D12_HEAP_TYPE heap_type,
                                             D3D12_BARRIER_LAYOUT initial_layout,
                                             D3D12_CLEAR_VALUE const* clear_value) -> SharedDeviceChildHandle<Texture>;
  [[nodiscard]] LEOPPHAPI auto CreatePipelineState(PipelineDesc const& desc,
                                                   std::uint8_t num_32_bit_params) -> SharedDeviceChildHandle<
    PipelineState>;
  [[nodiscard]] LEOPPHAPI auto CreateCommandList() -> SharedDeviceChildHandle<CommandList>;
  [[nodiscard]] LEOPPHAPI auto CreateFence(UINT64 initial_value) -> SharedDeviceChildHandle<Fence>;
  [[nodiscard]] LEOPPHAPI auto CreateSwapChain(SwapChainDesc const& desc,
                                               HWND window_handle) -> SharedDeviceChildHandle<SwapChain>;
  [[nodiscard]] LEOPPHAPI auto CreateSampler(D3D12_SAMPLER_DESC const& desc) -> UniqueSamplerHandle;
  LEOPPHAPI auto CreateAliasingResources(std::span<BufferDesc const> buffer_descs,
                                         std::span<AliasedTextureCreateInfo const> texture_infos,
                                         D3D12_HEAP_TYPE heap_type,
                                         std::pmr::vector<SharedDeviceChildHandle<Buffer>>* buffers,
                                         std::pmr::vector<SharedDeviceChildHandle<Texture>>* textures) -> void;

  LEOPPHAPI auto DestroyBuffer(Buffer const* buffer) -> void;
  LEOPPHAPI auto DestroyTexture(Texture const* texture) -> void;
  LEOPPHAPI auto DestroyPipelineState(PipelineState const* pipeline_state) -> void;
  LEOPPHAPI auto DestroyCommandList(CommandList const* command_list) -> void;
  LEOPPHAPI auto DestroyFence(Fence const* fence) -> void;
  LEOPPHAPI auto DestroySwapChain(SwapChain const* swap_chain) -> void;
  LEOPPHAPI auto DestroySampler(UINT sampler) -> void;

  [[nodiscard]] LEOPPHAPI auto WaitFence(Fence const& fence, UINT64 wait_value) const -> bool;
  [[nodiscard]] LEOPPHAPI auto SignalFence(Fence& fence, UINT64 signal_value) const -> bool;
  LEOPPHAPI auto ExecuteCommandLists(std::span<CommandList const> cmd_lists) const -> void;
  [[nodiscard]] LEOPPHAPI auto WaitIdle() const -> bool;

  [[nodiscard]] LEOPPHAPI auto SwapChainGetBuffers(
    SwapChain const& swap_chain) const -> std::span<SharedDeviceChildHandle<Texture> const>;
  [[nodiscard]] LEOPPHAPI auto SwapChainGetCurrentBufferIndex(SwapChain const& swap_chain) const -> UINT;
  [[nodiscard]] LEOPPHAPI auto SwapChainPresent(SwapChain const& swap_chain, UINT sync_interval) const -> bool;
  [[nodiscard]] LEOPPHAPI auto SwapChainResize(SwapChain& swap_chain, UINT width, UINT height) -> bool;

private:
  GraphicsDevice(Microsoft::WRL::ComPtr<IDXGIFactory7> factory, Microsoft::WRL::ComPtr<ID3D12Device10> device,
                 Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> res_desc_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sampler_heap,
                 Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue);

  auto SwapChainCreateTextures(SwapChain& swap_chain) -> bool;

  auto CreateBufferViews(ID3D12Resource2& buffer, BufferDesc const& desc, UINT& cbv, UINT& srv, UINT& uav) -> void;
  auto CreateTextureViews(ID3D12Resource2& texture, TextureDesc const& desc, UINT& dsv, UINT& rtv, UINT& srv,
                          UINT& uav) -> void;

  static UINT const rtv_heap_size_;
  static UINT const dsv_heap_size_;
  static UINT const res_desc_heap_size_;
  static UINT const sampler_heap_size_;

  Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
  Microsoft::WRL::ComPtr<ID3D12Device10> device_;
  Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator_;

  details::DescriptorHeap rtv_heap_;
  details::DescriptorHeap dsv_heap_;
  details::DescriptorHeap res_desc_heap_;
  details::DescriptorHeap sampler_heap_;

  Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;

  details::RootSignatureCache root_signatures_;

  UINT swap_chain_flags_{0};
  UINT present_flags_{0};

  SharedDeviceChildHandle<Fence> idle_fence_;
};


class Resource {
public:
  [[nodiscard]] LEOPPHAPI auto SetDebugName(std::wstring_view name) const -> bool;
  [[nodiscard]] LEOPPHAPI auto GetDesc() const -> D3D12_RESOURCE_DESC1;
  [[nodiscard]] LEOPPHAPI auto Map() const -> void*;
  [[nodiscard]] LEOPPHAPI auto GetShaderResource() const -> UINT;
  [[nodiscard]] LEOPPHAPI auto GetUnorderedAccess() const -> UINT;
  [[nodiscard]] LEOPPHAPI auto GetRequiredIntermediateSize() const -> UINT64;

protected:
  Resource(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
           UINT srv, UINT uav);

  [[nodiscard]] auto InternalMap(UINT subresource, D3D12_RANGE const* read_range) const -> void*;

private:
  Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation_;
  Microsoft::WRL::ComPtr<ID3D12Resource2> resource_;

  UINT srv_;
  UINT uav_;

  friend GraphicsDevice;
  friend CommandList;
};


class Buffer : public Resource {
public:
  [[nodiscard]] LEOPPHAPI auto GetConstantBuffer() const -> UINT;

private:
  Buffer(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
         UINT cbv, UINT srv, UINT uav);

  UINT cbv_;

  friend GraphicsDevice;
  friend CommandList;
};


class Texture : public Resource {
public:
  [[nodiscard]] LEOPPHAPI auto Map(UINT subresource) const -> void*;

private:
  Texture(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
          UINT dsv, UINT rtv, UINT srv, UINT uav);

  UINT dsv_;
  UINT rtv_;

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
  [[nodiscard]] LEOPPHAPI auto Begin(PipelineState const* pipeline_state) -> bool;
  [[nodiscard]] LEOPPHAPI auto End() const -> bool;
  LEOPPHAPI auto Barrier(std::span<GlobalBarrier const> global_barriers, std::span<BufferBarrier const> buffer_barriers,
                         std::span<TextureBarrier const> texture_barriers) const -> void;
  LEOPPHAPI auto ClearDepthStencil(Texture const& tex, D3D12_CLEAR_FLAGS clear_flags, FLOAT depth, UINT8 stencil,
                                   std::span<D3D12_RECT const> rects) const -> void;
  LEOPPHAPI auto ClearRenderTarget(Texture const& tex, std::span<FLOAT const, 4> color_rgba,
                                   std::span<D3D12_RECT const> rects) const -> void;
  LEOPPHAPI auto CopyBuffer(Buffer const& dst, Buffer const& src) const -> void;
  LEOPPHAPI auto CopyBufferRegion(Buffer const& dst, UINT64 dst_offset, Buffer const& src, UINT64 src_offset,
                                  UINT64 num_bytes) const -> void;
  LEOPPHAPI auto CopyTexture(Texture const& dst, Texture const& src) const -> void;
  LEOPPHAPI auto CopyTextureRegion(Texture const& dst, UINT dst_subresource_index, UINT dst_x, UINT dst_y, UINT dst_z,
                                   Texture const& src, UINT src_subresource_index,
                                   D3D12_BOX const* src_box) const -> void;
  LEOPPHAPI auto CopyTextureRegion(Texture const& dst, UINT dst_subresource_index, UINT dst_x, UINT dst_y, UINT dst_z,
                                   Buffer const& src,
                                   D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& src_footprint) const -> void;
  LEOPPHAPI auto Dispatch(UINT thread_group_count_x, UINT thread_group_count_y,
                          UINT thread_group_count_z) const -> void;
  LEOPPHAPI auto DispatchMesh(UINT thread_group_count_x, UINT thread_group_count_y,
                              UINT thread_group_count_z) const -> void;
  LEOPPHAPI auto DrawIndexedInstanced(UINT index_count_per_instance, UINT instance_count, UINT start_index_location,
                                      INT base_vertex_location, UINT start_instance_location) const -> void;
  LEOPPHAPI auto DrawInstanced(UINT vertex_count_per_instance, UINT instance_count, UINT start_vertex_location,
                               UINT start_instance_location) const -> void;
  LEOPPHAPI auto Resolve(Texture const& dst, Texture const& src, DXGI_FORMAT format) const -> void;
  LEOPPHAPI auto SetBlendFactor(std::span<FLOAT const, 4> blend_factor) const -> void;
  LEOPPHAPI auto SetIndexBuffer(Buffer const& buf, DXGI_FORMAT index_format) const -> void;
  LEOPPHAPI auto SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) const -> void;
  LEOPPHAPI auto SetRenderTargets(std::span<Texture const> render_targets, Texture const* depth_stencil) const -> void;
  LEOPPHAPI auto SetStencilRef(UINT stencil_ref) const -> void;
  LEOPPHAPI auto SetScissorRects(std::span<D3D12_RECT const> rects) const -> void;
  LEOPPHAPI auto SetViewports(std::span<D3D12_VIEWPORT const> viewports) const -> void;
  LEOPPHAPI auto SetPipelineParameter(UINT index, UINT value) const -> void;
  LEOPPHAPI auto SetPipelineParameters(UINT index, std::span<UINT const> values) const -> void;
  LEOPPHAPI auto SetPipelineState(PipelineState const& pipeline_state) -> void;
  LEOPPHAPI auto SetStreamOutputTargets(UINT start_slot,
                                        std::span<D3D12_STREAM_OUTPUT_BUFFER_VIEW const> views) const -> void;
  LEOPPHAPI auto UpdateSubresources(Resource const& dst, Buffer const& upload_buf, UINT64 buf_offset,
                                    UINT first_subresource, UINT num_subresources,
                                    D3D12_SUBRESOURCE_DATA const* src_data) const -> UINT64;

private:
  auto SetRootSignature(std::uint8_t num_params) const -> void;

  CommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator,
              Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmd_list, details::DescriptorHeap const* dsv_heap,
              details::DescriptorHeap const* rtv_heap, details::DescriptorHeap const* res_desc_heap,
              details::DescriptorHeap const* sampler_heap, details::RootSignatureCache* root_signatures);

  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator_;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmd_list_;
  details::DescriptorHeap const* dsv_heap_;
  details::DescriptorHeap const* rtv_heap_;
  details::DescriptorHeap const* res_desc_heap_;
  details::DescriptorHeap const* sampler_heap_;
  details::RootSignatureCache* root_signatures_;
  bool compute_pipeline_set_{false};

  friend GraphicsDevice;
};


class Fence {
public:
  [[nodiscard]] LEOPPHAPI auto GetNextValue() const -> UINT64;
  [[nodiscard]] LEOPPHAPI auto GetCompletedValue() const -> UINT64;
  [[nodiscard]] LEOPPHAPI auto Signal(UINT64 value) -> bool;
  [[nodiscard]] LEOPPHAPI auto Wait(UINT64 value) const -> bool;

private:
  explicit Fence(Microsoft::WRL::ComPtr<ID3D12Fence> fence, UINT64 next_value);

  Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
  std::atomic<UINT64> next_val_;

  friend GraphicsDevice;
};


class SwapChain {
  explicit SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain);

  Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_;
  std::vector<SharedDeviceChildHandle<Texture>> textures_;

  friend GraphicsDevice;
};


namespace details {
template<DeviceChild T>
class DeviceChildDeleter {
public:
  DeviceChildDeleter() = default;
  explicit DeviceChildDeleter(GraphicsDevice& device);
  auto operator()(T const* device_child) -> void;

private:
  GraphicsDevice* device_;
};


template<DeviceChild T>
DeviceChildDeleter<T>::DeviceChildDeleter(GraphicsDevice& device):
  device_{&device} {}


template<DeviceChild T>
auto DeviceChildDeleter<T>::operator()(T const* device_child) -> void {
  if (device_) {
    if constexpr (std::same_as<T, Buffer>) {
      device_->DestroyBuffer(device_child);
    } else if constexpr (std::same_as<T, Texture>) {
      device_->DestroyTexture(device_child);
    } else if constexpr (std::same_as<T, PipelineState>) {
      device_->DestroyPipelineState(device_child);
    } else if constexpr (std::same_as<T, CommandList>) {
      device_->DestroyCommandList(device_child);
    } else if constexpr (std::same_as<T, Fence>) {
      device_->DestroyFence(device_child);
    } else if constexpr (std::same_as<T, SwapChain>) {
      device_->DestroySwapChain(device_child);
    }
  }
}
}


class UniqueSamplerHandle {
public:
  UniqueSamplerHandle() = default;
  LEOPPHAPI UniqueSamplerHandle(UINT resource, GraphicsDevice& device);
  UniqueSamplerHandle(UniqueSamplerHandle const& other) = delete;
  LEOPPHAPI UniqueSamplerHandle(UniqueSamplerHandle&& other) noexcept;

  LEOPPHAPI ~UniqueSamplerHandle();

  auto operator=(UniqueSamplerHandle const& other) -> void = delete;
  LEOPPHAPI auto operator=(UniqueSamplerHandle&& other) noexcept -> UniqueSamplerHandle&;

  [[nodiscard]] LEOPPHAPI auto Get() const -> UINT;
  [[nodiscard]] LEOPPHAPI auto IsValid() const -> bool;

private:
  auto InternalDestruct() const -> void;

  UINT resource_{details::kInvalidResourceIndex};
  GraphicsDevice* device_{nullptr};
};
}
