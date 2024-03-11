#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "graphics.hpp"

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <D3D12MemAlloc.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <type_traits>
#include <string_view>
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
using Sampler = UINT;


namespace details {
UINT constexpr kInvalidResourceIndex{static_cast<UINT>(-1)};
}


template<typename T>
class UniqueHandle {
public:
  UniqueHandle() = default;
  UniqueHandle(T* resource, GraphicsDevice& device);
  UniqueHandle(UniqueHandle const& other) = delete;
  UniqueHandle(UniqueHandle&& other) noexcept;

  ~UniqueHandle();

  auto operator=(UniqueHandle const& other) -> void = delete;
  auto operator=(UniqueHandle&& other) noexcept -> UniqueHandle&;

  [[nodiscard]] auto Get() const -> T*;
  [[nodiscard]] auto IsValid() const -> bool;

  [[nodiscard]] auto operator*() const -> T&;
  [[nodiscard]] auto operator->() const -> T*;

private:
  auto InternalDestruct() const -> void;

  T* resource_{nullptr};
  GraphicsDevice* device_{nullptr};
};


template<>
class UniqueHandle<Sampler> {
public:
  UniqueHandle() = default;
  UniqueHandle(UINT resource, GraphicsDevice& device);
  UniqueHandle(UniqueHandle const& other) = delete;
  UniqueHandle(UniqueHandle&& other) noexcept;

  ~UniqueHandle();

  auto operator=(UniqueHandle const& other) -> void = delete;
  auto operator=(UniqueHandle&& other) noexcept -> UniqueHandle&;

  [[nodiscard]] auto Get() const -> UINT;
  [[nodiscard]] auto IsValid() const -> bool;

private:
  auto InternalDestruct() const -> void;

  UINT resource_{details::kInvalidResourceIndex};
  GraphicsDevice* device_{nullptr};
};


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
  [[nodiscard]] static auto New(bool enable_debug) -> std::unique_ptr<GraphicsDevice>;

  [[nodiscard]] auto CreateBuffer(BufferDesc const& desc, D3D12_HEAP_TYPE heap_type) -> UniqueHandle<Buffer>;
  [[nodiscard]] auto CreateTexture(TextureDesc const& desc, D3D12_HEAP_TYPE heap_type,
                                   D3D12_BARRIER_LAYOUT initial_layout,
                                   D3D12_CLEAR_VALUE const* clear_value) -> UniqueHandle<Texture>;
  [[nodiscard]] auto CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC const& desc, std::uint8_t num_32_bit_params,
                                         bool is_compute) -> UniqueHandle<PipelineState>;
  [[nodiscard]] auto CreateCommandList() -> UniqueHandle<CommandList>;
  [[nodiscard]] auto CreateFence(UINT64 initial_value) -> Microsoft::WRL::ComPtr<ID3D12Fence1>;
  [[nodiscard]] auto CreateSwapChain(SwapChainDesc const& desc, HWND window_handle) -> UniqueHandle<SwapChain>;
  [[nodiscard]] auto CreateSampler(D3D12_SAMPLER_DESC const& desc) -> UniqueHandle<Sampler>;
  auto CreateAliasingResources(std::span<BufferDesc const> buffer_descs,
                               std::span<AliasedTextureCreateInfo const> texture_infos, D3D12_HEAP_TYPE heap_type,
                               std::pmr::vector<UniqueHandle<Buffer>>* buffers,
                               std::pmr::vector<UniqueHandle<Texture>>* textures) -> void;

  auto DestroyBuffer(Buffer const* buffer) -> void;
  auto DestroyTexture(Texture const* texture) -> void;
  auto DestroyPipelineState(PipelineState const* pipeline_state) -> void;
  auto DestroyCommandList(CommandList const* command_list) -> void;
  auto DestroySwapChain(SwapChain const* swap_chain) -> void;
  auto DestroySampler(UINT sampler) -> void;

  [[nodiscard]] auto WaitFence(ID3D12Fence& fence, UINT64 wait_value) const -> bool;
  [[nodiscard]] auto SignalFence(ID3D12Fence& fence, UINT64 signal_value) const -> bool;
  auto ExecuteCommandLists(std::span<CommandList const> cmd_lists) -> bool;
  [[nodiscard]] auto WaitIdle() const -> bool;

  [[nodiscard]] auto SwapChainGetBuffers(SwapChain const& swap_chain) const -> std::span<UniqueHandle<Texture> const>;
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
                 Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue, Microsoft::WRL::ComPtr<ID3D12Fence> fence);

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

  Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
  std::atomic<UINT64> next_fence_val_{1};
};


class Resource {
public:
  [[nodiscard]] auto SetDebugName(std::wstring_view name) const -> bool;
  [[nodiscard]] auto Map() const -> void*;
  [[nodiscard]] auto GetShaderResource() const -> UINT;
  [[nodiscard]] auto GetUnorderedAccess() const -> UINT;

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
  [[nodiscard]] auto GetConstantBuffer() const -> UINT;

private:
  Buffer(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
         UINT cbv, UINT srv, UINT uav);

  UINT cbv_;

  friend GraphicsDevice;
  friend CommandList;
};


class Texture : public Resource {
public:
  [[nodiscard]] auto Map(UINT subresource) const -> void*;

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
  [[nodiscard]] auto Begin(PipelineState const* pipeline_state) -> bool;
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
  auto Resolve(Texture const& dst, Texture const& src, DXGI_FORMAT format) const -> void;
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


class SwapChain {
  explicit SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain);

  Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_;
  std::vector<UniqueHandle<Texture>> textures_;

  friend GraphicsDevice;
};


template<typename T>
UniqueHandle<T>::UniqueHandle(T* resource, GraphicsDevice& device) :
  resource_{resource},
  device_{&device} {}


template<typename T>
UniqueHandle<T>::UniqueHandle(UniqueHandle&& other) noexcept :
  resource_{other.resource_},
  device_{other.device_} {
  other.resource_ = nullptr;
  other.device_ = nullptr;
}


template<typename T>
UniqueHandle<T>::~UniqueHandle() {
  InternalDestruct();
}


template<typename T>
auto UniqueHandle<T>::operator=(UniqueHandle&& other) noexcept -> UniqueHandle& {
  if (this != &other) {
    InternalDestruct();
    resource_ = other.resource_;
    device_ = other.device_;
    other.resource_ = nullptr;
    other.device_ = nullptr;
  }
  return *this;
}


template<typename T>
auto UniqueHandle<T>::Get() const -> T* {
  return resource_;
}


template<typename T>
auto UniqueHandle<T>::IsValid() const -> bool {
  return resource_ != nullptr;
}


template<typename T>
auto UniqueHandle<T>::operator*() const -> T& {
  return *resource_;
}


template<typename T>
auto UniqueHandle<T>::operator->() const -> T* {
  return resource_;
}


template<typename T>
auto UniqueHandle<T>::InternalDestruct() const -> void {
  if (device_) {
    if constexpr (std::is_same_v<T, Buffer>) {
      device_->DestroyBuffer(resource_);
    } else if constexpr (std::is_same_v<T, Texture>) {
      device_->DestroyTexture(resource_);
    } else if constexpr (std::is_same_v<T, PipelineState>) {
      device_->DestroyPipelineState(resource_);
    } else if constexpr (std::is_same_v<T, CommandList>) {
      device_->DestroyCommandList(resource_);
    } else if constexpr (std::is_same_v<T, SwapChain>) {
      device_->DestroySwapChain(resource_);
    }
  }
}
}
