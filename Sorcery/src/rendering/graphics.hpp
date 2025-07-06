#pragma once

#include <atomic>
#include <concepts>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <D3D12MemAlloc.h>
#include <mimalloc.h>

#include "../Core.hpp"
#include "../fast_vector.hpp"

// Returns the index of the member if all the pipeline parameters are considered a single buffer of the specified type.
#define PIPELINE_PARAM_INDEX(BufferType, MemberName) static_cast<UINT>(offsetof(BufferType, MemberName) / 4)


namespace sorcery::graphics {
// Convert a depth format to its typeless equivalent.
[[nodiscard]] auto MakeDepthTypeless(DXGI_FORMAT depth_format) -> DXGI_FORMAT;
// Convert a depth format to its underlying linear equivalent.
[[nodiscard]] auto MakeDepthUnderlyingLinear(DXGI_FORMAT depth_format) -> DXGI_FORMAT;

class GraphicsDevice;

class Buffer;
class Texture;
class PipelineState;
class CommandList;
class Fence;
class SwapChain;
using Sampler = UINT;


namespace details {
auto constexpr kInvalidResourceIndex{static_cast<UINT>(-1)};

template<typename T>concept DeviceChild = std::same_as<std::remove_const_t<T>, Buffer> || std::same_as<
                                            std::remove_const_t<T>, Texture> || std::same_as<
                                            std::remove_const_t<T>, PipelineState> || std::same_as<
                                            std::remove_const_t<T>, CommandList> || std::same_as<
                                            std::remove_const_t<T>, Fence> || std::same_as<
                                            std::remove_const_t<T>, SwapChain>;

template<DeviceChild T>
class DeviceChildDeleter;
}


enum class CpuAccess : std::uint8_t {
  kNone  = 0,
  kRead  = 1,
  kWrite = 2
};


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
  UINT sample_count;

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
  FastVector<UINT> free_indices_;
  std::mutex mutex_;
  UINT increment_size_;
  UINT reserved_idx_count_;
  UINT heap_size_;
};


class RootSignatureCache {
public:
  auto Add(std::uint8_t num_params,
           Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature) -> Microsoft::WRL::ComPtr<ID3D12RootSignature>;
  [[nodiscard]] auto Get(std::uint8_t num_params) -> Microsoft::WRL::ComPtr<ID3D12RootSignature>;

private:
  std::unordered_map<std::uint8_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>, std::hash<std::uint8_t>, std::equal_to<
                       std::uint8_t>, mi_stl_allocator<std::pair<
                       std::uint8_t const, Microsoft::WRL::ComPtr<ID3D12RootSignature>>>> root_signatures_;
  std::mutex mutex_;
};


template<typename ResourceStateType>
class ResourceStateTracker {
public:
  auto Record(ID3D12Resource* const resource, ResourceStateType const state) -> void {
    resource_states_[resource] = state;
  }


  [[nodiscard]] auto Get(ID3D12Resource* const resource) const -> std::optional<ResourceStateType> {
    if (auto const it{resource_states_.find(resource)}; it != std::end(resource_states_)) {
      return it->second;
    }

    return std::nullopt;
  }


  auto Clear() -> void {
    resource_states_.clear();
  }


  auto begin() const {
    return std::begin(resource_states_);
  }


  auto end() const {
    return std::end(resource_states_);
  }

private:
  std::unordered_map<ID3D12Resource*, ResourceStateType, std::hash<ID3D12Resource*>, std::equal_to<ID3D12Resource*>,
                     mi_stl_allocator<std::pair<ID3D12Resource* const, ResourceStateType>>> resource_states_;
};


struct GlobalResourceState {
  D3D12_BARRIER_LAYOUT layout{D3D12_BARRIER_LAYOUT_UNDEFINED};
};


struct PipelineResourceState {
  D3D12_BARRIER_SYNC sync{D3D12_BARRIER_SYNC_NONE};
  D3D12_BARRIER_ACCESS access{D3D12_BARRIER_ACCESS_NO_ACCESS};
  D3D12_BARRIER_LAYOUT layout{D3D12_BARRIER_LAYOUT_UNDEFINED};
};


using GlobalResourceStateTracker = ResourceStateTracker<GlobalResourceState>;
using PipelineResourceStateTracker = ResourceStateTracker<PipelineResourceState>;


struct PendingBarrier {
  D3D12_BARRIER_LAYOUT layout;
  ID3D12Resource* resource;
};


struct ExecuteBarrierCmdListRecord {
  SharedDeviceChildHandle<CommandList> cmd_list;
  UINT64 fence_completion_val;
};
}


class GraphicsDevice {
public:
  LEOPPHAPI explicit GraphicsDevice(bool enable_debug);
  GraphicsDevice(GraphicsDevice const&) = delete;
  GraphicsDevice(GraphicsDevice&&) = delete;

  ~GraphicsDevice() = default;

  auto operator=(GraphicsDevice const&) -> void = delete;
  auto operator=(GraphicsDevice&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto CreateBuffer(BufferDesc const& desc,
                                            CpuAccess cpu_access) -> SharedDeviceChildHandle<Buffer>;
  [[nodiscard]] LEOPPHAPI auto CreateTexture(TextureDesc const& desc,
                                             CpuAccess cpu_access,
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
                                         CpuAccess cpu_access,
                                         FastVector<SharedDeviceChildHandle<Buffer>>* buffers,
                                         FastVector<SharedDeviceChildHandle<Texture>>* textures) -> void;

  LEOPPHAPI auto DestroyBuffer(Buffer const* buffer) const -> void;
  LEOPPHAPI auto DestroyTexture(Texture const* texture) const -> void;
  LEOPPHAPI auto DestroyPipelineState(PipelineState const* pipeline_state) const -> void;
  LEOPPHAPI auto DestroyCommandList(CommandList const* command_list) const -> void;
  LEOPPHAPI auto DestroyFence(Fence const* fence) const -> void;
  LEOPPHAPI auto DestroySwapChain(SwapChain const* swap_chain) const -> void;
  LEOPPHAPI auto DestroySampler(UINT sampler) const -> void;

  LEOPPHAPI auto WaitFence(Fence const& fence, UINT64 wait_value) const -> void;
  LEOPPHAPI auto SignalFence(Fence& fence) const -> void;
  LEOPPHAPI auto ExecuteCommandLists(std::span<CommandList const> cmd_lists) -> void;
  LEOPPHAPI auto WaitIdle() const -> void;

  LEOPPHAPI auto ResizeSwapChain(SwapChain& swap_chain, UINT width, UINT height) -> void;
  LEOPPHAPI auto Present(SwapChain const& swap_chain) -> void;

  LEOPPHAPI auto GetCopyableFootprints(TextureDesc const& desc, UINT first_subresource, UINT subresource_count,
                                       UINT64 base_offset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts,
                                       UINT* row_counts, UINT64* row_sizes, UINT64* total_size) const -> void;

private:
  auto SwapChainCreateTextures(SwapChain& swap_chain) -> void;

  auto CreateBufferViews(ID3D12Resource2& buffer, BufferDesc const& desc, UINT& cbv, UINT& srv,
                         UINT& uav) const -> void;
  auto CreateTextureViews(ID3D12Resource2& texture, TextureDesc const& desc, FastVector<UINT>& dsvs,
                          FastVector<UINT>& rtvs, std::optional<UINT>& srv,
                          std::optional<UINT>& uav) const -> void;

  [[nodiscard]] auto AcquirePendingBarrierCmdList() -> CommandList&;

  [[nodiscard]] auto MakeHeapType(CpuAccess cpu_access) const -> D3D12_HEAP_TYPE;

  static UINT const rtv_heap_size_;
  static UINT const dsv_heap_size_;
  static UINT const res_desc_heap_size_;
  static UINT const sampler_heap_size_;

  Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
  Microsoft::WRL::ComPtr<ID3D12Device10> device_;
  Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator_;

  std::unique_ptr<details::DescriptorHeap> rtv_heap_;
  std::unique_ptr<details::DescriptorHeap> dsv_heap_;
  std::unique_ptr<details::DescriptorHeap> res_desc_heap_;
  std::unique_ptr<details::DescriptorHeap> sampler_heap_;

  Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;

  details::RootSignatureCache root_signatures_;
  details::GlobalResourceStateTracker global_resource_states_;

  UINT swap_chain_flags_{0};
  UINT present_flags_{0};

  SharedDeviceChildHandle<Fence> idle_fence_;
  SharedDeviceChildHandle<Fence> execute_barrier_fence_;

  FastVector<details::ExecuteBarrierCmdListRecord> execute_barrier_cmd_lists_;
  std::mutex execute_barrier_mutex_;

  CD3DX12FeatureSupport supported_features_;
};


class Resource {
public:
  LEOPPHAPI auto SetDebugName(std::wstring_view name) const -> void;
  [[nodiscard]]
  LEOPPHAPI auto Map() const -> void*;
  LEOPPHAPI auto Unmap() const -> void;
  [[nodiscard]]
  LEOPPHAPI auto GetShaderResource() const -> UINT;
  [[nodiscard]]
  LEOPPHAPI auto GetUnorderedAccess() const -> UINT;
  [[nodiscard]]
  LEOPPHAPI auto GetInternalResource() const -> ID3D12Resource2*;

protected:
  Resource(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
           std::optional<UINT> srv, std::optional<UINT> uav);

  [[nodiscard]] auto InternalMap(UINT subresource, D3D12_RANGE const* read_range) const -> void*;
  auto InternalUnmap(UINT subresource, D3D12_RANGE const* written_range) const -> void;

private:
  Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation_;
  Microsoft::WRL::ComPtr<ID3D12Resource2> resource_;

  std::optional<UINT> srv_;
  std::optional<UINT> uav_;

  friend GraphicsDevice;
};


class Buffer : public Resource {
public:
  [[nodiscard]]
  LEOPPHAPI auto GetDesc() const -> BufferDesc const&;
  [[nodiscard]]
  LEOPPHAPI auto GetConstantBuffer() const -> UINT;

private:
  Buffer(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
         std::optional<UINT> cbv, std::optional<UINT> srv, std::optional<UINT> uav, BufferDesc const& desc);

  BufferDesc desc_;
  std::optional<UINT> cbv_;

  friend GraphicsDevice;
};


class Texture : public Resource {
public:
  [[nodiscard]]
  LEOPPHAPI auto GetDesc() const -> TextureDesc const&;
  [[nodiscard]]
  LEOPPHAPI auto Map(UINT subresource) const -> void*;
  LEOPPHAPI auto Unmap(UINT subresource) const -> void;
  [[nodiscard]]
  LEOPPHAPI auto GetDepthStencilView(UINT mip_index) const -> UINT;
  [[nodiscard]]
  LEOPPHAPI auto GetRenderTargetView(UINT mip_index) const -> UINT;

private:
  Texture(Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation, Microsoft::WRL::ComPtr<ID3D12Resource2> resource,
          FastVector<UINT> dsvs, FastVector<UINT> rtvs, std::optional<UINT> srv, std::optional<UINT> uav,
          TextureDesc const& desc);

  TextureDesc desc_;
  // One per mip
  FastVector<UINT> dsvs_;
  // One per mip
  FastVector<UINT> rtvs_;

  friend GraphicsDevice;
};


class PipelineState {
  PipelineState(Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature,
                Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state, std::uint8_t num_params, bool is_compute,
                bool allows_ds_write);

  Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_;
  std::uint8_t num_params_;
  bool is_compute_;
  bool allows_ds_write_;

  friend GraphicsDevice;
  friend CommandList;
};


class CommandList {
public:
  LEOPPHAPI auto Begin(PipelineState const* pipeline_state) -> void;
  LEOPPHAPI auto End() const -> void;
  LEOPPHAPI auto ClearDepthStencil(Texture const& tex, D3D12_CLEAR_FLAGS clear_flags, FLOAT depth, UINT8 stencil,
                                   std::span<D3D12_RECT const> rects, UINT16 mip_level = 0) -> void;
  LEOPPHAPI auto ClearRenderTarget(Texture const& tex, std::span<FLOAT const, 4> color_rgba,
                                   std::span<D3D12_RECT const> rects, UINT16 mip_level = 0) -> void;
  LEOPPHAPI auto CopyBuffer(Buffer const& dst, Buffer const& src) -> void;
  LEOPPHAPI auto CopyBufferRegion(Buffer const& dst, UINT64 dst_offset, Buffer const& src, UINT64 src_offset,
                                  UINT64 num_bytes) -> void;
  LEOPPHAPI auto CopyTexture(Texture const& dst, Texture const& src) -> void;
  LEOPPHAPI auto CopyTextureRegion(Texture const& dst, UINT dst_subresource_index, UINT dst_x, UINT dst_y, UINT dst_z,
                                   Texture const& src, UINT src_subresource_index, D3D12_BOX const* src_box) -> void;
  LEOPPHAPI auto CopyTextureRegion(Texture const& dst, UINT dst_subresource_index, UINT dst_x, UINT dst_y, UINT dst_z,
                                   Buffer const& src, D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& src_footprint) -> void;
  LEOPPHAPI auto DiscardRenderTarget(Texture const& tex, std::optional<D3D12_DISCARD_REGION> const& region) -> void;
  LEOPPHAPI auto DiscardDepthStencil(Texture const& tex, std::optional<D3D12_DISCARD_REGION> const& region) -> void;
  LEOPPHAPI auto Dispatch(UINT thread_group_count_x, UINT thread_group_count_y,
                          UINT thread_group_count_z) const -> void;
  LEOPPHAPI auto DispatchMesh(UINT thread_group_count_x, UINT thread_group_count_y,
                              UINT thread_group_count_z) const -> void;
  LEOPPHAPI auto DrawIndexedInstanced(UINT index_count_per_instance, UINT instance_count, UINT start_index_location,
                                      INT base_vertex_location, UINT start_instance_location) const -> void;
  LEOPPHAPI auto DrawInstanced(UINT vertex_count_per_instance, UINT instance_count, UINT start_vertex_location,
                               UINT start_instance_location) const -> void;
  LEOPPHAPI auto Resolve(Texture const& dst, Texture const& src, DXGI_FORMAT format) -> void;
  LEOPPHAPI auto SetBlendFactor(std::span<FLOAT const, 4> blend_factor) const -> void;
  LEOPPHAPI auto SetIndexBuffer(Buffer const& buf, DXGI_FORMAT index_format) -> void;
  LEOPPHAPI auto SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) const -> void;
  LEOPPHAPI auto SetRenderTargets(std::span<Texture const* const> render_targets, Texture const* depth_stencil,
                                  UINT16 mip_level = 0) -> void;
  LEOPPHAPI auto SetStencilRef(UINT stencil_ref) const -> void;
  LEOPPHAPI auto SetScissorRects(std::span<D3D12_RECT const> rects) const -> void;
  LEOPPHAPI auto SetViewports(std::span<D3D12_VIEWPORT const> viewports) const -> void;
  LEOPPHAPI auto SetPipelineParameter(UINT index, UINT value) const -> void;
  LEOPPHAPI auto SetPipelineParameters(UINT index, std::span<UINT const> values) const -> void;
  LEOPPHAPI auto SetConstantBuffer(UINT param_idx, Buffer const& buf) -> void;
  LEOPPHAPI auto SetShaderResource(UINT param_idx, Buffer const& buf) -> void;
  LEOPPHAPI auto SetShaderResource(UINT param_idx, Texture const& tex) -> void;
  LEOPPHAPI auto SetUnorderedAccess(UINT param_idx, Buffer const& buf) -> void;
  LEOPPHAPI auto SetUnorderedAccess(UINT param_idx, Texture const& tex) -> void;
  LEOPPHAPI auto SetPipelineState(PipelineState const& pipeline_state) -> void;

private:
  auto SetRootSignature(std::uint8_t num_params) const -> void;

  CommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator,
              Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmd_list, details::DescriptorHeap const* dsv_heap,
              details::DescriptorHeap const* rtv_heap, details::DescriptorHeap const* res_desc_heap,
              details::DescriptorHeap const* sampler_heap, details::RootSignatureCache* root_signatures);

  auto GenerateBarrier(Buffer const& buf, D3D12_BARRIER_SYNC sync, D3D12_BARRIER_ACCESS access) -> void;
  auto GenerateBarrier(Texture const& tex, D3D12_BARRIER_SYNC sync, D3D12_BARRIER_ACCESS access,
                       D3D12_BARRIER_LAYOUT layout) -> void;

  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator_;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmd_list_;
  details::PipelineResourceStateTracker local_resource_states_;
  FastVector<details::PendingBarrier> pending_barriers_;
  details::DescriptorHeap const* dsv_heap_;
  details::DescriptorHeap const* rtv_heap_;
  details::DescriptorHeap const* res_desc_heap_;
  details::DescriptorHeap const* sampler_heap_;
  details::RootSignatureCache* root_signatures_;
  bool compute_pipeline_set_{false};
  bool pipeline_allows_ds_write_{false};

  friend GraphicsDevice;
};


class Fence {
public:
  [[nodiscard]] LEOPPHAPI auto GetNextValue() const -> UINT64;
  [[nodiscard]] LEOPPHAPI auto GetCompletedValue() const -> UINT64;
  LEOPPHAPI auto Wait(UINT64 wait_value) const -> void;
  LEOPPHAPI auto Signal() -> void;

private:
  explicit Fence(Microsoft::WRL::ComPtr<ID3D12Fence> fence, UINT64 next_value);

  Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
  std::atomic<UINT64> next_val_;

  friend GraphicsDevice;
};


class SwapChain {
public:
  [[nodiscard]] LEOPPHAPI auto GetTextures() const -> std::span<SharedDeviceChildHandle<Texture const> const>;
  [[nodiscard]] LEOPPHAPI auto GetCurrentTextureIndex() const -> UINT;
  [[nodiscard]] LEOPPHAPI auto GetCurrentTexture() const -> Texture const&;

  [[nodiscard]] LEOPPHAPI auto GetSyncInterval() const -> UINT;
  LEOPPHAPI auto SetSyncInterval(UINT sync_interval) -> void;

private:
  explicit SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain, UINT present_flags);

  Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain_;
  FastVector<SharedDeviceChildHandle<Texture>> textures_;
  std::atomic<UINT> sync_interval_{0};
  UINT present_flags_;

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
