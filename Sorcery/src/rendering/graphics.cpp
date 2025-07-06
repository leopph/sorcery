#include "graphics.hpp"

#include "../MemoryAllocation.hpp"

#include <dxgidebug.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "../Util.hpp"

using Microsoft::WRL::ComPtr;


namespace sorcery::graphics {
UINT const GraphicsDevice::rtv_heap_size_{1'000'000};
UINT const GraphicsDevice::dsv_heap_size_{1'000'000};
UINT const GraphicsDevice::res_desc_heap_size_{1'000'000};
UINT const GraphicsDevice::sampler_heap_size_{2048};


namespace {
auto ThrowIfFailed(HRESULT const hr, std::string_view const msg) -> void {
  if (FAILED(hr)) {
    throw std::runtime_error{msg.data()};
  }
}


auto AsD3d12Desc(BufferDesc const& desc) -> D3D12_RESOURCE_DESC1 {
  auto flags{D3D12_RESOURCE_FLAG_NONE};

  if (!desc.shader_resource) {
    flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
  }

  if (desc.unordered_access) {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  return CD3DX12_RESOURCE_DESC1::Buffer(desc.size, flags);
}


auto AsD3d12Desc(TextureDesc const& desc) -> D3D12_RESOURCE_DESC1 {
  auto flags{D3D12_RESOURCE_FLAG_NONE};

  if (desc.depth_stencil) {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  }

  if (desc.render_target) {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }

  if (!desc.shader_resource) {
    flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
  }

  if (desc.unordered_access) {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  switch (desc.dimension) {
    case TextureDimension::k1D: {
      return CD3DX12_RESOURCE_DESC1::Tex1D(desc.format, desc.width, desc.depth_or_array_size, desc.mip_levels, flags);
    }
    case TextureDimension::k2D: [[fallthrough]];
    case TextureDimension::kCube: {
      return CD3DX12_RESOURCE_DESC1::Tex2D(desc.format, desc.width, desc.height, desc.depth_or_array_size,
        desc.mip_levels, desc.sample_count, 0, flags);
    }
    case TextureDimension::k3D: {
      return CD3DX12_RESOURCE_DESC1::Tex3D(desc.format, desc.width, desc.height, desc.depth_or_array_size,
        desc.mip_levels, flags);
    }
  }

  throw std::runtime_error{"Trying to convert invalid an TextureDesc to D3D12_RESOURCE_DESC1."};
}
}


namespace details {
auto DescriptorHeap::Allocate() -> UINT {
  std::scoped_lock const lock{mutex_};

  if (free_indices_.empty()) {
    auto const old_reserve_count{reserved_idx_count_};
    reserved_idx_count_ = std::min(heap_size_, reserved_idx_count_ * 2);

    if (old_reserve_count == reserved_idx_count_) {
      throw std::runtime_error{"Failed to allocate descriptor heap indices: the heap is full."};
    }

    free_indices_.reserve(reserved_idx_count_ - old_reserve_count);

    for (UINT idx{old_reserve_count}; idx < reserved_idx_count_; idx++) {
      free_indices_.emplace_back(idx);
    }

    // TODO start descriptor heap itself small, grow it on demand
  }

  auto const idx{free_indices_.back()};
  free_indices_.pop_back();
  return idx;
}


auto DescriptorHeap::Release(UINT const index) -> void {
  if (index == kInvalidResourceIndex) {
    return;
  }

  std::scoped_lock const lock{mutex_};
  free_indices_.insert(std::ranges::upper_bound(free_indices_, index), index);
  free_indices_.erase(std::ranges::unique(free_indices_).begin(), free_indices_.end());
}


auto DescriptorHeap::GetDescriptorCpuHandle(UINT const descriptor_index) const -> D3D12_CPU_DESCRIPTOR_HANDLE {
  if (descriptor_index >= reserved_idx_count_) {
    throw std::runtime_error{"Failed to convert descriptor index to CPU handle: descriptor index is out of range."};
  }

  return CD3DX12_CPU_DESCRIPTOR_HANDLE{
    heap_->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptor_index), increment_size_
  };
}


auto DescriptorHeap::GetDescriptorGpuHandle(UINT const descriptor_index) const -> D3D12_GPU_DESCRIPTOR_HANDLE {
  if (descriptor_index >= reserved_idx_count_) {
    throw std::runtime_error{"Failed to convert descriptor index to GPU handle: descriptor index is out of range."};
  }

  return CD3DX12_GPU_DESCRIPTOR_HANDLE{
    heap_->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptor_index), increment_size_
  };
}


auto DescriptorHeap::GetInternalPtr() const -> ID3D12DescriptorHeap* {
  return heap_.Get();
}


DescriptorHeap::DescriptorHeap(ComPtr<ID3D12DescriptorHeap> heap, ID3D12Device& device) :
  heap_{std::move(heap)},
  increment_size_{device.GetDescriptorHandleIncrementSize(heap_->GetDesc().Type)},
  reserved_idx_count_{1},
  heap_size_{heap_->GetDesc().NumDescriptors} {
  free_indices_.reserve(reserved_idx_count_);
  for (UINT idx{0}; idx < reserved_idx_count_; idx++) {
    free_indices_.emplace_back(idx);
  }
}


auto RootSignatureCache::Add(std::uint8_t const num_params,
                             ComPtr<ID3D12RootSignature> root_signature) -> ComPtr<ID3D12RootSignature> {
  std::scoped_lock const lock{mutex_};
  return root_signatures_.try_emplace(num_params, std::move(root_signature)).first->second;
}


auto RootSignatureCache::Get(std::uint8_t const num_params) -> ComPtr<ID3D12RootSignature> {
  std::scoped_lock const lock{mutex_};

  if (auto const it{root_signatures_.find(num_params)}; it != std::end(root_signatures_)) {
    return it->second;
  }

  return nullptr;
}
}


auto MakeDepthTypeless(DXGI_FORMAT const depth_format) -> DXGI_FORMAT {
  if (depth_format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
    return DXGI_FORMAT_R32G8X24_TYPELESS;
  }

  if (depth_format == DXGI_FORMAT_D32_FLOAT) {
    return DXGI_FORMAT_R32_TYPELESS;
  }

  if (depth_format == DXGI_FORMAT_D24_UNORM_S8_UINT) {
    return DXGI_FORMAT_R24G8_TYPELESS;
  }

  if (depth_format == DXGI_FORMAT_D16_UNORM) {
    return DXGI_FORMAT_R16_TYPELESS;
  }

  return depth_format;
}


auto MakeDepthUnderlyingLinear(DXGI_FORMAT const depth_format) -> DXGI_FORMAT {
  if (depth_format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
  }

  if (depth_format == DXGI_FORMAT_D32_FLOAT) {
    return DXGI_FORMAT_R32_FLOAT;
  }

  if (depth_format == DXGI_FORMAT_D24_UNORM_S8_UINT) {
    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  }

  if (depth_format == DXGI_FORMAT_D16_UNORM) {
    return DXGI_FORMAT_R16_UNORM;
  }

  return depth_format;
}


auto GetActualMipLevels(TextureDesc const& desc) -> UINT {
  return desc.mip_levels == 0
           ? static_cast<UINT16>(std::ceil(std::max(std::log2(desc.width), std::log2(desc.height)) + 1))
           : desc.mip_levels;
}


GraphicsDevice::GraphicsDevice(bool const enable_debug) {
  if (enable_debug) {
    ComPtr<ID3D12Debug6> debug;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)), "Failed to get D3D12 debug interface.");
    debug->EnableDebugLayer();

    ComPtr<IDXGIInfoQueue> dxgi_info_queue;
    ThrowIfFailed(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_info_queue)), "Failed to get DXGI debug interface.");
    ThrowIfFailed(dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE),
      "Failed to set debug break on DXGI error.");
    ThrowIfFailed(
      dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE),
      "Failed to set debug break on DXGI corruption.");
  }

  UINT factory_create_flags{0};

  if (enable_debug) {
    factory_create_flags |= DXGI_CREATE_FACTORY_DEBUG;
  }

  ThrowIfFailed(CreateDXGIFactory2(factory_create_flags, IID_PPV_ARGS(&factory_)), "Failed to create DXGI factory.");

  ComPtr<IDXGIAdapter4> adapter;
  ThrowIfFailed(factory_->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)),
    "Failed to get high performance adapter.");

  ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_)),
    "Failed to create D3D12 device.");

  if (enable_debug) {
    ComPtr<ID3D12InfoQueue> d3d12_info_queue;
    ThrowIfFailed(device_.As(&d3d12_info_queue), "Failed to get D3D12 info queue.");
    ThrowIfFailed(d3d12_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE),
      "Failed to set debug break on D3D12 error.");
    ThrowIfFailed(d3d12_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE),
      "Failed to set debug break on D3D12 corruption.");
  }

  ThrowIfFailed(supported_features_.Init(device_.Get()), "Failed to query D3D12 features.");

  if (supported_features_.ResourceBindingTier() < D3D12_RESOURCE_BINDING_TIER_3) {
    throw std::runtime_error{"Resource Bindig Tier 3 is required but not supported."};
  }

  if (supported_features_.HighestShaderModel() < D3D_SHADER_MODEL_6_6) {
    throw std::runtime_error{"Shader Model 6.6 is required but not supported."};
  }

  if (!supported_features_.EnhancedBarriersSupported()) {
    throw std::runtime_error{"Enhanced barriers is required but not supported."};
  }

  if (supported_features_.HighestRootSignatureVersion() < D3D_ROOT_SIGNATURE_VERSION_1_1) {
    throw std::runtime_error{"Root Signature 1.1 is required but no supported."};
  }

  if (!supported_features_.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation()) {
    throw std::runtime_error{
      "Viewport and render target array index outside of geometry shaders is required but no supported."
    };
  }

  if (supported_features_.MeshShaderTier() < D3D12_MESH_SHADER_TIER_1) {
    throw std::runtime_error{"Mesh Shader Tier 1 is required but not supported."};
  }

  D3D12MA::ALLOCATOR_DESC const allocator_desc{D3D12MA::ALLOCATOR_FLAG_NONE, device_.Get(), 0, nullptr, adapter.Get()};
  ThrowIfFailed(CreateAllocator(&allocator_desc, &allocator_), "Failed to create D3D12 memory allocator.");

  D3D12_DESCRIPTOR_HEAP_DESC constexpr rtv_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtv_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0
  };
  ComPtr<ID3D12DescriptorHeap> rtv_heap;
  ThrowIfFailed(device_->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)), "Failed to create RTV heap.");
  rtv_heap_ = std::make_unique<details::DescriptorHeap>(std::move(rtv_heap), *device_.Get());

  D3D12_DESCRIPTOR_HEAP_DESC constexpr dsv_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsv_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0
  };
  ComPtr<ID3D12DescriptorHeap> dsv_heap;
  ThrowIfFailed(device_->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap)), "Failed to create DSV heap.");
  dsv_heap_ = std::make_unique<details::DescriptorHeap>(std::move(dsv_heap), *device_.Get());

  D3D12_DESCRIPTOR_HEAP_DESC constexpr res_desc_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, res_desc_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0
  };
  ComPtr<ID3D12DescriptorHeap> res_desc_heap;
  ThrowIfFailed(device_->CreateDescriptorHeap(&res_desc_heap_desc, IID_PPV_ARGS(&res_desc_heap)),
    "Failed to create resource descriptor heap.");
  res_desc_heap_ = std::make_unique<details::DescriptorHeap>(std::move(res_desc_heap), *device_.Get());

  D3D12_DESCRIPTOR_HEAP_DESC constexpr sampler_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, sampler_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0
  };
  ComPtr<ID3D12DescriptorHeap> sampler_heap;
  ThrowIfFailed(device_->CreateDescriptorHeap(&sampler_heap_desc, IID_PPV_ARGS(&sampler_heap)),
    "Failed to create sampler heap.");
  sampler_heap_ = std::make_unique<details::DescriptorHeap>(std::move(sampler_heap), *device_.Get());

  D3D12_COMMAND_QUEUE_DESC constexpr queue_desc{
    D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE, 0
  };
  ThrowIfFailed(device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&queue_)), "Failed to create command queue.");

  if (BOOL allow_tearing; SUCCEEDED(
    factory_->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing)
    )) && allow_tearing) {
    swap_chain_flags_ |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    present_flags_ |= DXGI_PRESENT_ALLOW_TEARING;
  }

  idle_fence_ = CreateFence(0);
  execute_barrier_fence_ = CreateFence(0);
}


auto GraphicsDevice::CreateBuffer(BufferDesc const& desc,
                                  CpuAccess const cpu_access) -> SharedDeviceChildHandle<Buffer> {
  ComPtr<D3D12MA::Allocation> allocation;
  ComPtr<ID3D12Resource2> resource;

  D3D12MA::ALLOCATION_DESC const alloc_desc{
    D3D12MA::ALLOCATION_FLAG_NONE, MakeHeapType(cpu_access), D3D12_HEAP_FLAG_NONE, nullptr, nullptr
  };

  auto const res_desc{AsD3d12Desc(desc)};

  ThrowIfFailed(allocator_->CreateResource3(&alloc_desc, &res_desc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr,
    &allocation, IID_PPV_ARGS(&resource)), "Failed to create buffer.");

  UINT cbv;
  UINT srv;
  UINT uav;

  CreateBufferViews(*resource.Get(), desc, cbv, srv, uav);

  global_resource_states_.Record(resource.Get(), {.layout = D3D12_BARRIER_LAYOUT_UNDEFINED});

  return SharedDeviceChildHandle<Buffer>{
    new Buffer{std::move(allocation), std::move(resource), cbv, srv, uav, desc},
    details::DeviceChildDeleter<Buffer>{*this}
  };
}


auto GraphicsDevice::CreateTexture(TextureDesc const& desc,
                                   CpuAccess const cpu_access,
                                   D3D12_CLEAR_VALUE const* clear_value) -> SharedDeviceChildHandle<Texture> {
  ComPtr<D3D12MA::Allocation> allocation;
  ComPtr<ID3D12Resource2> resource;

  auto res_desc{AsD3d12Desc(desc)};

  // If a depth format is specified, we have to determine the typeless resource format.
  res_desc.Format = MakeDepthTypeless(desc.format);

  D3D12MA::ALLOCATION_DESC const alloc_desc{
    D3D12MA::ALLOCATION_FLAG_NONE, MakeHeapType(cpu_access), D3D12_HEAP_FLAG_NONE, nullptr, nullptr
  };

  constexpr auto initial_layout{D3D12_BARRIER_LAYOUT_UNDEFINED};

  ThrowIfFailed(allocator_->CreateResource3(&alloc_desc, &res_desc, initial_layout, clear_value, 0, nullptr,
    &allocation, IID_PPV_ARGS(&resource)), "Failed to create texture.");

  FastVector<UINT> dsvs;
  FastVector<UINT> rtvs;
  std::optional<UINT> srv;
  std::optional<UINT> uav;

  CreateTextureViews(*resource.Get(), desc, dsvs, rtvs, srv, uav);

  global_resource_states_.Record(resource.Get(), {.layout = initial_layout});

  return SharedDeviceChildHandle<Texture>{
    new Texture{std::move(allocation), std::move(resource), std::move(dsvs), std::move(rtvs), srv, uav, desc},
    details::DeviceChildDeleter<Texture>{*this}
  };
}


auto GraphicsDevice::CreatePipelineState(PipelineDesc const& desc,
                                         std::uint8_t const num_32_bit_params) -> SharedDeviceChildHandle<
  PipelineState> {
  auto root_signature{root_signatures_.Get(num_32_bit_params)};

  if (!root_signature) {
    std::array<CD3DX12_ROOT_PARAMETER1, 2> root_params;
    root_params[0].InitAsConstants(num_32_bit_params, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
    // 2 params: base vertex and base instance. Make sure this aligns with the shader code!
    root_params[1].InitAsConstants(2, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC const root_signature_desc{
      .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
      .Desc_1_1 = {
        static_cast<UINT>(root_params.size()), root_params.data(), 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
        D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
      }
    };

    ComPtr<ID3DBlob> root_signature_blob;
    ComPtr<ID3DBlob> error_blob;

    ThrowIfFailed(D3D12SerializeVersionedRootSignature(&root_signature_desc, &root_signature_blob, &error_blob),
      "Failed to serialize root signature.");
    ThrowIfFailed(device_->CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
      root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&root_signature)), "Failed to create root signature.");

    root_signature = root_signatures_.Add(num_32_bit_params, std::move(root_signature));
  }

  ComPtr<ID3D12PipelineState> pipeline_state;

  CD3DX12_PIPELINE_STATE_STREAM2 pso_desc;
  pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
  pso_desc.NodeMask = 0;
  pso_desc.pRootSignature = root_signature.Get();
  pso_desc.PrimitiveTopologyType = desc.primitive_topology_type;
  pso_desc.VS = desc.vs;
  pso_desc.GS = desc.gs;
  pso_desc.StreamOutput = desc.stream_output;
  pso_desc.HS = desc.hs;
  pso_desc.DS = desc.ds;
  pso_desc.PS = desc.ps;
  pso_desc.AS = desc.as;
  pso_desc.MS = desc.ms;
  pso_desc.CS = desc.cs;
  pso_desc.BlendState = desc.blend_state;
  pso_desc.DepthStencilState = desc.depth_stencil_state;
  pso_desc.DSVFormat = desc.ds_format;
  pso_desc.RasterizerState = desc.rasterizer_state;
  pso_desc.RTVFormats = desc.rt_formats;
  pso_desc.SampleDesc = desc.sample_desc;
  pso_desc.SampleMask = desc.sample_mask;
  pso_desc.ViewInstancingDesc = desc.view_instancing_desc;

  D3D12_PIPELINE_STATE_STREAM_DESC const stream_desc{sizeof(pso_desc), &pso_desc};
  ThrowIfFailed(device_->CreatePipelineState(&stream_desc, IID_PPV_ARGS(&pipeline_state)),
    "Failed to create pipeline state.");

  return SharedDeviceChildHandle<PipelineState>{
    new PipelineState{
      std::move(root_signature), std::move(pipeline_state), num_32_bit_params, desc.cs.BytecodeLength != 0,
      desc.depth_stencil_state.DepthEnable && desc.depth_stencil_state.DepthWriteMask != D3D12_DEPTH_WRITE_MASK_ZERO
    },
    details::DeviceChildDeleter<PipelineState>{*this}
  };
}


auto GraphicsDevice::CreateCommandList() -> SharedDeviceChildHandle<CommandList> {
  ComPtr<ID3D12CommandAllocator> allocator;
  ThrowIfFailed(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)),
    "Failed to create command allocator.");

  ComPtr<ID3D12GraphicsCommandList7> cmd_list;
  ThrowIfFailed(
    device_->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE,
      IID_PPV_ARGS(&cmd_list)), "Failed to create command list.");

  return SharedDeviceChildHandle<CommandList>{
    new CommandList{
      std::move(allocator), std::move(cmd_list), dsv_heap_.get(), rtv_heap_.get(), res_desc_heap_.get(),
      sampler_heap_.get(), &root_signatures_
    },
    details::DeviceChildDeleter<CommandList>{*this}
  };
}


auto GraphicsDevice::CreateFence(UINT64 const initial_value) -> SharedDeviceChildHandle<Fence> {
  ComPtr<ID3D12Fence1> fence;
  ThrowIfFailed(device_->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)),
    "Failed to create fence.");
  return SharedDeviceChildHandle<Fence>{
    new Fence{std::move(fence), initial_value + 1}, details::DeviceChildDeleter<Fence>{*this}
  };
}


auto GraphicsDevice::CreateSwapChain(SwapChainDesc const& desc,
                                     HWND const window_handle) -> SharedDeviceChildHandle<SwapChain> {
  DXGI_SWAP_CHAIN_DESC1 const dxgi_desc{
    desc.width, desc.height, desc.format, FALSE, {1, 0}, desc.usage, desc.buffer_count, desc.scaling,
    DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_UNSPECIFIED, swap_chain_flags_
  };

  ComPtr<IDXGISwapChain1> swap_chain1;
  ThrowIfFailed(
    factory_->CreateSwapChainForHwnd(queue_.Get(), window_handle, &dxgi_desc, nullptr, nullptr, &swap_chain1),
    "Failed to create swap chain.");

  ComPtr<IDXGISwapChain4> swap_chain4;
  ThrowIfFailed(swap_chain1.As(&swap_chain4), "Failed to query IDXGISwapChain4 interface.");
  ThrowIfFailed(factory_->MakeWindowAssociation(window_handle, DXGI_MWA_NO_ALT_ENTER),
    "Failed to disable swap chain ALT+ENTER behavior.");

  auto const swap_chain{new SwapChain{std::move(swap_chain4), present_flags_}};
  SwapChainCreateTextures(*swap_chain);

  return SharedDeviceChildHandle<SwapChain>{swap_chain, details::DeviceChildDeleter<SwapChain>{*this}};
}


auto GraphicsDevice::CreateSampler(D3D12_SAMPLER_DESC const& desc) -> UniqueSamplerHandle {
  auto const sampler{sampler_heap_->Allocate()};
  device_->CreateSampler(&desc, sampler_heap_->GetDescriptorCpuHandle(sampler));
  return UniqueSamplerHandle{sampler, *this};
}


auto GraphicsDevice::CreateAliasingResources(std::span<BufferDesc const> const buffer_descs,
                                             std::span<AliasedTextureCreateInfo const> const texture_infos,
                                             CpuAccess cpu_access,
                                             FastVector<SharedDeviceChildHandle<Buffer>>* buffers,
                                             FastVector<SharedDeviceChildHandle<Texture>>* textures) -> void {
  D3D12_RESOURCE_ALLOCATION_INFO buf_alloc_info{0, 0};
  D3D12_RESOURCE_ALLOCATION_INFO rt_ds_alloc_info{0, 0};
  D3D12_RESOURCE_ALLOCATION_INFO non_rt_ds_alloc_info{0, 0};

  for (auto const& buffer_desc : buffer_descs) {
    auto const desc{AsD3d12Desc(buffer_desc)};
    auto const alloc_info{device_->GetResourceAllocationInfo2(0, 1, &desc, nullptr)};
    buf_alloc_info.Alignment = std::max(buf_alloc_info.Alignment, alloc_info.Alignment);
    buf_alloc_info.SizeInBytes = std::max(buf_alloc_info.SizeInBytes, alloc_info.SizeInBytes);
  }

  for (auto const& info : texture_infos) {
    auto const& desc{AsD3d12Desc(info.desc)};
    auto const alloc_info{device_->GetResourceAllocationInfo2(0, 1, &desc, nullptr)};
    auto& tex_alloc_info{info.desc.render_target || info.desc.depth_stencil ? rt_ds_alloc_info : non_rt_ds_alloc_info};

    tex_alloc_info.Alignment = std::max(tex_alloc_info.Alignment, alloc_info.Alignment);
    tex_alloc_info.SizeInBytes = std::max(tex_alloc_info.SizeInBytes, alloc_info.SizeInBytes);
  }

  auto const heap_type{MakeHeapType(cpu_access)};

  ComPtr<D3D12MA::Allocation> buf_alloc;
  ComPtr<D3D12MA::Allocation> rt_ds_alloc;
  ComPtr<D3D12MA::Allocation> non_rt_ds_alloc;

  if (allocator_->GetD3D12Options().ResourceHeapTier > D3D12_RESOURCE_HEAP_TIER_1) {
    D3D12MA::ALLOCATION_DESC alloc_desc{
      D3D12MA::ALLOCATION_FLAG_NONE, heap_type, D3D12_HEAP_FLAG_NONE, nullptr, nullptr
    };

    if (buf_alloc_info.SizeInBytes == 0) {
      alloc_desc.ExtraHeapFlags |= D3D12_HEAP_FLAG_DENY_BUFFERS;
    }

    if (rt_ds_alloc_info.SizeInBytes == 0) {
      alloc_desc.ExtraHeapFlags |= D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
    }

    if (non_rt_ds_alloc_info.SizeInBytes == 0) {
      alloc_desc.ExtraHeapFlags |= D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
    }

    D3D12_RESOURCE_ALLOCATION_INFO const alloc_info{
      std::max(std::max(buf_alloc_info.SizeInBytes, rt_ds_alloc_info.SizeInBytes), non_rt_ds_alloc_info.SizeInBytes),
      std::max(std::max(buf_alloc_info.Alignment, rt_ds_alloc_info.Alignment), non_rt_ds_alloc_info.Alignment)
    };

    ThrowIfFailed(allocator_->AllocateMemory(&alloc_desc, &alloc_info, &buf_alloc),
      "Failed to allocate memory for aliasing resources.");

    rt_ds_alloc = buf_alloc;
    non_rt_ds_alloc = buf_alloc;
  } else {
    if (buf_alloc_info.SizeInBytes > 0) {
      D3D12MA::ALLOCATION_DESC const buf_alloc_desc{
        D3D12MA::ALLOCATION_FLAG_NONE, heap_type, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, nullptr, nullptr
      };

      ThrowIfFailed(allocator_->AllocateMemory(&buf_alloc_desc, &buf_alloc_info, &buf_alloc),
        "Failed to allocate memory for aliasing buffers.");
    }

    if (rt_ds_alloc_info.SizeInBytes > 0) {
      D3D12MA::ALLOCATION_DESC const rt_ds_alloc_desc{
        D3D12MA::ALLOCATION_FLAG_NONE, heap_type, D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, nullptr, nullptr
      };

      ThrowIfFailed(allocator_->AllocateMemory(&rt_ds_alloc_desc, &rt_ds_alloc_info, &rt_ds_alloc),
        "Failed to allocate memory for aliasing RT/DS textures.");
    }

    if (non_rt_ds_alloc_info.SizeInBytes > 0) {
      D3D12MA::ALLOCATION_DESC const non_rt_ds_alloc_desc{
        D3D12MA::ALLOCATION_FLAG_NONE, heap_type, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES, nullptr, nullptr
      };

      ThrowIfFailed(allocator_->AllocateMemory(&non_rt_ds_alloc_desc, &non_rt_ds_alloc_info, &non_rt_ds_alloc),
        "Failed to allocate memory for aliasing non-RT/DS textures.");
    }
  }

  if (buffers) {
    for (auto const& buf_desc : buffer_descs) {
      auto const desc{AsD3d12Desc(buf_desc)};
      ComPtr<ID3D12Resource2> resource;

      ThrowIfFailed(allocator_->CreateAliasingResource2(buf_alloc.Get(), 0, &desc, D3D12_BARRIER_LAYOUT_UNDEFINED,
        nullptr, 0, nullptr, IID_PPV_ARGS(&resource)), "Failed to create aliasing buffer.");

      UINT cbv;
      UINT srv;
      UINT uav;
      CreateBufferViews(*resource.Get(), buf_desc, cbv, srv, uav);
      buffers->emplace_back(new Buffer{buf_alloc, std::move(resource), cbv, srv, uav, buf_desc},
        details::DeviceChildDeleter<Buffer>{*this});
    }
  }

  if (textures) {
    for (auto const& info : texture_infos) {
      auto const desc{AsD3d12Desc(info.desc)};

      auto& alloc{info.desc.render_target || info.desc.depth_stencil ? rt_ds_alloc : non_rt_ds_alloc};

      ComPtr<ID3D12Resource2> resource;
      ThrowIfFailed(allocator_->CreateAliasingResource2(alloc.Get(), 0, &desc, info.initial_layout, info.clear_value, 0,
        nullptr, IID_PPV_ARGS(&resource)), "Failed to create aliasing texture.");

      FastVector<UINT> dsvs;
      FastVector<UINT> rtvs;
      std::optional<UINT> srv;
      std::optional<UINT> uav;
      CreateTextureViews(*resource.Get(), info.desc, dsvs, rtvs, srv, uav);
      textures->emplace_back(new Texture{
        alloc, std::move(resource), std::move(dsvs), std::move(rtvs), srv, uav, info.desc
      }, details::DeviceChildDeleter<Texture>{*this});
    }
  }
}


auto GraphicsDevice::DestroyBuffer(Buffer const* const buffer) const -> void {
  if (buffer) {
    if (buffer->cbv_) {
      res_desc_heap_->Release(*buffer->cbv_);
    }

    if (buffer->srv_) {
      res_desc_heap_->Release(*buffer->srv_);
    }

    if (buffer->uav_) {
      res_desc_heap_->Release(*buffer->uav_);
    }

    delete buffer;
  }
}


auto GraphicsDevice::DestroyTexture(Texture const* const texture) const -> void {
  if (texture) {
    std::ranges::for_each(texture->dsvs_, [this](UINT const dsv) {
      dsv_heap_->Release(dsv);
    });

    std::ranges::for_each(texture->rtvs_, [this](UINT const rtv) {
      rtv_heap_->Release(rtv);
    });

    if (texture->srv_) {
      res_desc_heap_->Release(*texture->srv_);
    }

    if (texture->uav_) {
      res_desc_heap_->Release(*texture->uav_);
    }

    delete texture;
  }
}


auto GraphicsDevice::DestroyPipelineState(PipelineState const* const pipeline_state) const -> void {
  delete pipeline_state;
}


auto GraphicsDevice::DestroyCommandList(CommandList const* const command_list) const -> void {
  delete command_list;
}


auto GraphicsDevice::DestroyFence(Fence const* const fence) const -> void {
  delete fence;
}


auto GraphicsDevice::DestroySwapChain(SwapChain const* const swap_chain) const -> void {
  delete swap_chain;
}


auto GraphicsDevice::DestroySampler(UINT const sampler) const -> void {
  sampler_heap_->Release(sampler);
}


auto GraphicsDevice::WaitFence(Fence const& fence, UINT64 const wait_value) const -> void {
  ThrowIfFailed(queue_->Wait(fence.fence_.Get(), wait_value), "Failed to wait fence from GPU queue.");
}


auto GraphicsDevice::SignalFence(Fence& fence) const -> void {
  auto const new_fence_val{fence.next_val_.load()};
  ThrowIfFailed(queue_->Signal(fence.fence_.Get(), new_fence_val), "Failed to signal fence from GPU queue.");
  fence.next_val_ = new_fence_val + 1;
}


auto GraphicsDevice::ExecuteCommandLists(std::span<CommandList const> const cmd_lists) -> void {
  FastVector<D3D12_TEXTURE_BARRIER> pending_tex_barriers;

  for (auto const& cmd_list : cmd_lists) {
    for (auto const& pending_barrier : cmd_list.pending_barriers_) {
      auto const global_state{global_resource_states_.Get(pending_barrier.resource)};
      auto layout_before{global_state ? global_state->layout : D3D12_BARRIER_LAYOUT_UNDEFINED};

      pending_tex_barriers.emplace_back(D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_SYNC_NONE,
        D3D12_BARRIER_ACCESS_NO_ACCESS, D3D12_BARRIER_ACCESS_NO_ACCESS,
        layout_before, pending_barrier.layout, pending_barrier.resource,
        D3D12_BARRIER_SUBRESOURCE_RANGE{
          .IndexOrFirstMipLevel = 0xffffffff, .NumMipLevels = 0, .FirstArraySlice = 0, .NumArraySlices = 0,
          .FirstPlane = 0, .NumPlanes = 0
        }, D3D12_TEXTURE_BARRIER_FLAG_NONE);
    }

    for (auto const& [res, state] : cmd_list.local_resource_states_) {
      global_resource_states_.Record(res, {.layout = state.layout});
    }
  }

  D3D12_BARRIER_GROUP const pending_barrier_group{
    .Type = D3D12_BARRIER_TYPE_TEXTURE, .NumBarriers = clamp_cast<UINT32>(pending_tex_barriers.size()),
    .pTextureBarriers = pending_tex_barriers.data()
  };

  auto& pending_barrier_cmd{AcquirePendingBarrierCmdList()};

  pending_barrier_cmd.Begin(nullptr);
  pending_barrier_cmd.cmd_list_->Barrier(1, &pending_barrier_group);
  pending_barrier_cmd.End();
  queue_->ExecuteCommandLists(1,
    std::array{static_cast<ID3D12CommandList*>(pending_barrier_cmd.cmd_list_.Get())}.data());
  SignalFence(*execute_barrier_fence_);

  FastVector<ID3D12CommandList*> submit_list;
  submit_list.reserve(cmd_lists.size());
  std::ranges::transform(cmd_lists, std::back_inserter(submit_list), [](CommandList const& cmd_list) {
    return cmd_list.cmd_list_.Get();
  });
  queue_->ExecuteCommandLists(static_cast<UINT>(submit_list.size()), submit_list.data());
}


auto GraphicsDevice::WaitIdle() const -> void {
  auto const fence_val{idle_fence_->GetNextValue()};
  SignalFence(*idle_fence_);
  idle_fence_->Wait(fence_val);
}


auto GraphicsDevice::ResizeSwapChain(SwapChain& swap_chain, UINT const width, UINT const height) -> void {
  swap_chain.textures_.clear();
  ThrowIfFailed(swap_chain.swap_chain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, swap_chain_flags_),
    "Failed to resize swap chain buffers.");
  SwapChainCreateTextures(swap_chain);
}


auto GraphicsDevice::Present(SwapChain const& swap_chain) -> void {
  auto const cur_tex{swap_chain.GetCurrentTexture().resource_.Get()};
  auto const state{global_resource_states_.Get(cur_tex)};
  auto const layout_before{state ? state->layout : D3D12_BARRIER_LAYOUT_UNDEFINED};

  if (!state || state->layout != D3D12_BARRIER_LAYOUT_PRESENT) {
    global_resource_states_.Record(cur_tex, {.layout = D3D12_BARRIER_LAYOUT_PRESENT});

    D3D12_TEXTURE_BARRIER const barrier{
      D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_SYNC_NONE,
      D3D12_BARRIER_ACCESS_NO_ACCESS, D3D12_BARRIER_ACCESS_NO_ACCESS,
      layout_before, D3D12_BARRIER_LAYOUT_PRESENT,
      swap_chain.GetCurrentTexture().resource_.Get(),
      {
        .IndexOrFirstMipLevel = 0xffffffff, .NumMipLevels = 0, .FirstArraySlice = 0, .NumArraySlices = 0,
        .FirstPlane = 0, .NumPlanes = 0
      },
      D3D12_TEXTURE_BARRIER_FLAG_NONE
    };

    D3D12_BARRIER_GROUP const barrier_group{
      .Type = D3D12_BARRIER_TYPE_TEXTURE, .NumBarriers = 1, .pTextureBarriers = &barrier
    };

    auto& cmd_list{AcquirePendingBarrierCmdList()};
    cmd_list.Begin(nullptr);
    cmd_list.cmd_list_->Barrier(1, &barrier_group);
    cmd_list.End();

    queue_->ExecuteCommandLists(1, std::array{static_cast<ID3D12CommandList*>(cmd_list.cmd_list_.Get())}.data());
    SignalFence(*execute_barrier_fence_);
  }

  ThrowIfFailed(swap_chain.swap_chain_->Present(swap_chain.GetSyncInterval(), present_flags_),
    "Failed to present swap chain.");
}


auto GraphicsDevice::GetCopyableFootprints(TextureDesc const& desc, UINT const first_subresource,
                                           UINT const subresource_count, UINT64 const base_offset,
                                           D3D12_PLACED_SUBRESOURCE_FOOTPRINT* const layouts, UINT* const row_counts,
                                           UINT64* const row_sizes, UINT64* const total_size) const -> void {
  auto const tex_desc{AsD3d12Desc(desc)};
  return device_->GetCopyableFootprints1(&tex_desc, first_subresource, subresource_count, base_offset, layouts,
    row_counts, row_sizes, total_size);
}


auto GraphicsDevice::SwapChainCreateTextures(SwapChain& swap_chain) -> void {
  DXGI_SWAP_CHAIN_DESC1 desc;
  ThrowIfFailed(swap_chain.swap_chain_->GetDesc1(&desc), "Failed to retrieve swap chain desc.");

  TextureDesc const tex_desc{
    TextureDimension::k2D, desc.Width, desc.Height, 1, 1, desc.Format, 1, false,
    (desc.BufferUsage & DXGI_USAGE_RENDER_TARGET_OUTPUT) != 0, (desc.BufferUsage & DXGI_USAGE_SHADER_INPUT) != 0, false
  };

  for (UINT i{0}; i < desc.BufferCount; i++) {
    ComPtr<ID3D12Resource2> buf;
    ThrowIfFailed(swap_chain.swap_chain_->GetBuffer(i, IID_PPV_ARGS(&buf)), "Failed to retrieve swap chain buffer.");

    FastVector<UINT> dsvs;
    FastVector<UINT> rtvs;
    std::optional<UINT> srv;
    std::optional<UINT> uav;

    CreateTextureViews(*buf.Get(), tex_desc, dsvs, rtvs, srv, uav);

    global_resource_states_.Record(buf.Get(), {.layout = D3D12_BARRIER_LAYOUT_COMMON});

    swap_chain.textures_.emplace_back(new Texture{
      nullptr, std::move(buf), {}, std::move(rtvs), srv, details::kInvalidResourceIndex, tex_desc
    }, details::DeviceChildDeleter<Texture>{*this});
  }
}


auto GraphicsDevice::CreateBufferViews(ID3D12Resource2& buffer, BufferDesc const& desc, UINT& cbv, UINT& srv,
                                       UINT& uav) const -> void {
  if (desc.constant_buffer) {
    cbv = res_desc_heap_->Allocate();
    D3D12_CONSTANT_BUFFER_VIEW_DESC const cbv_desc{buffer.GetGPUVirtualAddress(), static_cast<UINT>(desc.size)};
    device_->CreateConstantBufferView(&cbv_desc, res_desc_heap_->GetDescriptorCpuHandle(cbv));
  } else {
    cbv = details::kInvalidResourceIndex;
  }

  if (desc.shader_resource) {
    srv = res_desc_heap_->Allocate();
    D3D12_SHADER_RESOURCE_VIEW_DESC const srv_desc{
      .Format = desc.stride == 1 ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN,
      .ViewDimension = D3D12_SRV_DIMENSION_BUFFER, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
      .Buffer = {
        0, static_cast<UINT>(desc.size / (desc.stride == 1 ? 4 : desc.stride)), desc.stride == 1 ? 0 : desc.stride,
        desc.stride == 1 ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE
      }
    };
    device_->CreateShaderResourceView(&buffer, &srv_desc, res_desc_heap_->GetDescriptorCpuHandle(srv));
  } else {
    srv = details::kInvalidResourceIndex;
  }

  if (desc.unordered_access) {
    uav = res_desc_heap_->Allocate();
    D3D12_UNORDERED_ACCESS_VIEW_DESC const uav_desc{
      .Format = desc.stride == 1 ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN,
      .ViewDimension = D3D12_UAV_DIMENSION_BUFFER, .Buffer = {
        .FirstElement = 0, .NumElements = static_cast<UINT>(desc.size / (desc.stride == 1 ? 4 : desc.stride)),
        .StructureByteStride = desc.stride == 1 ? 0 : desc.stride, .CounterOffsetInBytes = 0,
        .Flags = desc.stride == 1 ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE
      }
    };
    device_->CreateUnorderedAccessView(&buffer, nullptr, &uav_desc, res_desc_heap_->GetDescriptorCpuHandle(uav));
  } else {
    uav = details::kInvalidResourceIndex;
  }
}


auto GraphicsDevice::CreateTextureViews(ID3D12Resource2& texture, TextureDesc const& desc, FastVector<UINT>& dsvs,
                                        FastVector<UINT>& rtvs, std::optional<UINT>& srv,
                                        std::optional<UINT>& uav) const -> void {
  DXGI_FORMAT dsv_format;
  DXGI_FORMAT rtv_srv_uav_format;

  // If a depth format is specified, we have to determine the rtv/srv/uav format.
  if (desc.format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
    dsv_format = desc.format;
    rtv_srv_uav_format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
  } else if (desc.format == DXGI_FORMAT_D32_FLOAT) {
    dsv_format = desc.format;
    rtv_srv_uav_format = DXGI_FORMAT_R32_FLOAT;
  } else if (desc.format == DXGI_FORMAT_D24_UNORM_S8_UINT) {
    dsv_format = desc.format;
    rtv_srv_uav_format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  } else if (desc.format == DXGI_FORMAT_D16_UNORM) {
    dsv_format = desc.format;
    rtv_srv_uav_format = DXGI_FORMAT_R16_UNORM;
  } else {
    dsv_format = desc.format;
    rtv_srv_uav_format = desc.format;
  }

  auto const actual_mip_levels{GetActualMipLevels(desc)};

  if (desc.depth_stencil) {
    dsvs.reserve(actual_mip_levels);

    for (UINT16 i{0}; i < actual_mip_levels; ++i) {
      D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{.Format = dsv_format, .Flags = D3D12_DSV_FLAG_NONE};

      if (desc.dimension == TextureDimension::k1D) {
        if (desc.depth_or_array_size == 1) {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
          dsv_desc.Texture1D.MipSlice = i;
        } else {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
          dsv_desc.Texture1DArray.MipSlice = i;
          dsv_desc.Texture1DArray.FirstArraySlice = 0;
          dsv_desc.Texture1DArray.ArraySize = desc.depth_or_array_size;
        }
      } else if (desc.dimension == TextureDimension::k2D && desc.depth_or_array_size == 1) {
        if (desc.sample_count == 1) {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
          dsv_desc.Texture2D.MipSlice = i;
        } else {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        }
      } else if ((desc.dimension == TextureDimension::k2D && desc.depth_or_array_size > 1) || desc.dimension ==
                 TextureDimension::kCube) {
        if (desc.sample_count == 1) {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
          dsv_desc.Texture2DArray.MipSlice = i;
          dsv_desc.Texture2DArray.FirstArraySlice = 0;
          dsv_desc.Texture2DArray.ArraySize = desc.depth_or_array_size;
        } else {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
          dsv_desc.Texture2DMSArray.FirstArraySlice = 0;
          dsv_desc.Texture2DMSArray.ArraySize = desc.depth_or_array_size;
        }
      } else {
        throw std::runtime_error{"Cannot create depth stencil view for texture."};
      }

      dsvs.emplace_back(dsv_heap_->Allocate());
      device_->CreateDepthStencilView(&texture, &dsv_desc, dsv_heap_->GetDescriptorCpuHandle(dsvs.back()));
    }
  }

  if (desc.render_target) {
    rtvs.reserve(actual_mip_levels);

    for (UINT16 i{0}; i < actual_mip_levels; ++i) {
      D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{.Format = rtv_srv_uav_format};

      if (desc.dimension == TextureDimension::k1D) {
        if (desc.depth_or_array_size == 1) {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
          rtv_desc.Texture1D.MipSlice = i;
        } else {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
          rtv_desc.Texture1DArray.MipSlice = i;
          rtv_desc.Texture1DArray.FirstArraySlice = 0;
          rtv_desc.Texture1DArray.ArraySize = desc.depth_or_array_size;
        }
      } else if (desc.dimension == TextureDimension::k2D && desc.depth_or_array_size == 1) {
        if (desc.sample_count == 1) {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
          rtv_desc.Texture2D.MipSlice = i;
          rtv_desc.Texture2D.PlaneSlice = 0;
        } else {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        }
      } else if ((desc.dimension == TextureDimension::k2D && desc.depth_or_array_size > 1) || desc.dimension ==
                 TextureDimension::kCube) {
        if (desc.sample_count == 1) {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
          rtv_desc.Texture2DArray.MipSlice = i;
          rtv_desc.Texture2DArray.FirstArraySlice = 0;
          rtv_desc.Texture2DArray.ArraySize = desc.depth_or_array_size;
          rtv_desc.Texture2DArray.PlaneSlice = 0;
        } else {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
          rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
          rtv_desc.Texture2DMSArray.ArraySize = desc.depth_or_array_size;
        }
      } else if (desc.dimension == TextureDimension::k3D) {
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
        rtv_desc.Texture3D.MipSlice = i;
        rtv_desc.Texture3D.FirstWSlice = 0;
        rtv_desc.Texture3D.WSize = static_cast<UINT>(-1);
      } else {
        throw std::runtime_error{"Cannot create render target view for texture."};
      }

      rtvs.emplace_back(rtv_heap_->Allocate());
      device_->CreateRenderTargetView(&texture, &rtv_desc, rtv_heap_->GetDescriptorCpuHandle(rtvs.back()));
    }
  }

  if (desc.shader_resource) {
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{
      .Format = rtv_srv_uav_format, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
    };
    if (desc.dimension == TextureDimension::k1D) {
      if (desc.depth_or_array_size == 1) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        srv_desc.Texture1D.MostDetailedMip = 0;
        srv_desc.Texture1D.MipLevels = static_cast<UINT>(-1);
        srv_desc.Texture1D.ResourceMinLODClamp = 0.0f;
      } else {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        srv_desc.Texture1DArray.MostDetailedMip = 0;
        srv_desc.Texture1DArray.MipLevels = static_cast<UINT>(-1);
        srv_desc.Texture1DArray.FirstArraySlice = 0;
        srv_desc.Texture1DArray.ArraySize = desc.depth_or_array_size;
        srv_desc.Texture1DArray.ResourceMinLODClamp = 0.0f;
      }
    } else if (desc.dimension == TextureDimension::k2D) {
      if (desc.depth_or_array_size == 1) {
        if (desc.sample_count == 1) {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
          srv_desc.Texture2D.MostDetailedMip = 0;
          srv_desc.Texture2D.MipLevels = static_cast<UINT>(-1);
          srv_desc.Texture2D.PlaneSlice = 0;
          srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
        } else {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
      } else {
        if (desc.sample_count == 1) {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
          srv_desc.Texture2DArray.MostDetailedMip = 0;
          srv_desc.Texture2DArray.MipLevels = static_cast<UINT>(-1);
          srv_desc.Texture2DArray.FirstArraySlice = 0;
          srv_desc.Texture2DArray.ArraySize = desc.depth_or_array_size;
          srv_desc.Texture2DArray.PlaneSlice = 0;
          srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        } else {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
          srv_desc.Texture2DMSArray.FirstArraySlice = 0;
          srv_desc.Texture2DMSArray.ArraySize = desc.depth_or_array_size;
        }
      }
    } else if (desc.dimension == TextureDimension::k3D) {
      srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
      srv_desc.Texture3D.MostDetailedMip = 0;
      srv_desc.Texture3D.MipLevels = static_cast<UINT>(-1);
      srv_desc.Texture3D.ResourceMinLODClamp = 0.0f;
    } else if (desc.dimension == TextureDimension::kCube) {
      if (desc.depth_or_array_size == 6) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srv_desc.TextureCube.MostDetailedMip = 0;
        srv_desc.TextureCube.MipLevels = static_cast<UINT>(-1);
        srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
      } else {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        srv_desc.TextureCubeArray.MostDetailedMip = 0;
        srv_desc.TextureCubeArray.MipLevels = static_cast<UINT>(-1);
        srv_desc.TextureCubeArray.First2DArrayFace = 0;
        srv_desc.TextureCubeArray.NumCubes = desc.depth_or_array_size / 6;
        srv_desc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
      }
    } else {
      throw std::runtime_error{"Cannot create shader resource view for texture."};
    }
    srv = res_desc_heap_->Allocate();
    device_->CreateShaderResourceView(&texture, &srv_desc, res_desc_heap_->GetDescriptorCpuHandle(*srv));
  }

  if (desc.unordered_access) {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{.Format = rtv_srv_uav_format};
    if (desc.dimension == TextureDimension::k1D) {
      if (desc.depth_or_array_size == 1) {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        uav_desc.Texture1D.MipSlice = 0;
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        uav_desc.Texture1DArray.MipSlice = 0;
        uav_desc.Texture1DArray.FirstArraySlice = 0;
        uav_desc.Texture1DArray.ArraySize = desc.depth_or_array_size;
      }
    } else if (desc.dimension == TextureDimension::k2D && desc.depth_or_array_size == 1) {
      if (desc.sample_count == 1) {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        uav_desc.Texture2D.PlaneSlice = 0;
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMS;
      }
    } else if ((desc.dimension == TextureDimension::k2D && desc.depth_or_array_size > 1) || desc.dimension ==
               TextureDimension::k3D) {
      if (desc.sample_count == 1) {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uav_desc.Texture2DArray.MipSlice = 0;
        uav_desc.Texture2DArray.FirstArraySlice = 0;
        uav_desc.Texture2DArray.ArraySize = desc.depth_or_array_size;
        uav_desc.Texture2DArray.PlaneSlice = 0;
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY;
        uav_desc.Texture2DMSArray.FirstArraySlice = 0;
        uav_desc.Texture2DMSArray.ArraySize = desc.depth_or_array_size;
      }
    } else if (desc.dimension == TextureDimension::k3D) {
      uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
      uav_desc.Texture3D.MipSlice = 0;
      uav_desc.Texture3D.FirstWSlice = 0;
      uav_desc.Texture3D.WSize = static_cast<UINT>(-1);
    } else {
      throw std::runtime_error{"Cannot create unordered access view for texture."};
    }
    uav = res_desc_heap_->Allocate();
    device_->CreateUnorderedAccessView(&texture, nullptr, &uav_desc, res_desc_heap_->GetDescriptorCpuHandle(*uav));
  }
}


auto GraphicsDevice::AcquirePendingBarrierCmdList() -> CommandList& {
  std::unique_lock const lock{execute_barrier_mutex_};

  auto const completed_fence_val{execute_barrier_fence_->GetCompletedValue()};
  auto const next_fence_val{execute_barrier_fence_->GetNextValue()};

  for (std::size_t i{0}; i < execute_barrier_cmd_lists_.size(); i++) {
    if (execute_barrier_cmd_lists_[i].fence_completion_val <= completed_fence_val) {
      execute_barrier_cmd_lists_[i].fence_completion_val = next_fence_val;
      return *execute_barrier_cmd_lists_[i].cmd_list;
    }
  }

  return *execute_barrier_cmd_lists_.emplace_back(CreateCommandList(), next_fence_val).cmd_list;
}


auto GraphicsDevice::MakeHeapType(CpuAccess const cpu_access) const -> D3D12_HEAP_TYPE {
  switch (cpu_access) {
    case CpuAccess::kNone:
      return D3D12_HEAP_TYPE_DEFAULT;
    case CpuAccess::kRead:
      return D3D12_HEAP_TYPE_READBACK;
    case CpuAccess::kWrite:
      return supported_features_.GPUUploadHeapSupported() ? D3D12_HEAP_TYPE_GPU_UPLOAD : D3D12_HEAP_TYPE_UPLOAD;
  }

  throw std::runtime_error{"Failed to make D3D12 heap type: unknown CPU access type."};
}


auto Resource::SetDebugName(std::wstring_view const name) const -> void {
  ThrowIfFailed(resource_->SetName(name.data()), "Failed to set D3D12 resource debug name.");
}


auto Resource::Map() const -> void* {
  return InternalMap(0, nullptr);
}


auto Resource::Unmap() const -> void {
  InternalUnmap(0, nullptr);
}


auto Resource::GetShaderResource() const -> UINT {
  return srv_.value();
}


auto Resource::GetUnorderedAccess() const -> UINT {
  return uav_.value();
}


auto Resource::GetInternalResource() const -> ID3D12Resource2* {
  return resource_.Get();
}


Resource::Resource(ComPtr<D3D12MA::Allocation> allocation, ComPtr<ID3D12Resource2> resource,
                   std::optional<UINT> const srv, std::optional<UINT> const uav) :
  allocation_{std::move(allocation)},
  resource_{std::move(resource)},
  srv_{srv},
  uav_{uav} {}


auto Resource::InternalMap(UINT const subresource, D3D12_RANGE const* read_range) const -> void* {
  void* mapped;
  ThrowIfFailed(resource_->Map(subresource, read_range, &mapped), "Failed to map D3D12 resource.");
  return mapped;
}


auto Resource::InternalUnmap(UINT const subresource, D3D12_RANGE const* written_range) const -> void {
  resource_->Unmap(subresource, written_range);
}


auto Buffer::GetDesc() const -> BufferDesc const& {
  return desc_;
}


auto Buffer::GetConstantBuffer() const -> UINT {
  return cbv_.value();
}


Buffer::Buffer(ComPtr<D3D12MA::Allocation> allocation, ComPtr<ID3D12Resource2> resource, std::optional<UINT> const cbv,
               std::optional<UINT> const srv, std::optional<UINT> const uav, BufferDesc const& desc) :
  Resource{std::move(allocation), std::move(resource), srv, uav},
  desc_{desc},
  cbv_{cbv} {}


auto Texture::GetDesc() const -> TextureDesc const& {
  return desc_;
}


auto Texture::Map(UINT const subresource) const -> void* {
  return InternalMap(subresource, nullptr);
}


auto Texture::Unmap(UINT const subresource) const -> void {
  InternalUnmap(subresource, nullptr);
}


auto Texture::GetDepthStencilView(UINT const mip_index) const -> UINT {
  return dsvs_.at(mip_index);
}


auto Texture::GetRenderTargetView(UINT const mip_index) const -> UINT {
  return rtvs_.at(mip_index);
}


Texture::Texture(ComPtr<D3D12MA::Allocation> allocation, ComPtr<ID3D12Resource2> resource, FastVector<UINT> dsvs,
                 FastVector<UINT> rtvs, std::optional<UINT> const srv, std::optional<UINT> const uav,
                 TextureDesc const& desc) :
  Resource{std::move(allocation), std::move(resource), srv, uav},
  desc_{desc},
  dsvs_{std::move(dsvs)},
  rtvs_{std::move(rtvs)} {}


PipelineState::PipelineState(ComPtr<ID3D12RootSignature> root_signature, ComPtr<ID3D12PipelineState> pipeline_state,
                             std::uint8_t const num_params, bool const is_compute, bool const allows_ds_write) :
  root_signature_{std::move(root_signature)},
  pipeline_state_{std::move(pipeline_state)},
  num_params_{num_params},
  is_compute_{is_compute},
  allows_ds_write_{allows_ds_write} {}


auto CommandList::Begin(PipelineState const* pipeline_state) -> void {
  ThrowIfFailed(allocator_->Reset(), "Failed to reset command allocator.");
  ThrowIfFailed(cmd_list_->Reset(allocator_.Get(), pipeline_state ? pipeline_state->pipeline_state_.Get() : nullptr),
    "Failed to reset command list.");
  cmd_list_->SetDescriptorHeaps(2,
    std::array{res_desc_heap_->GetInternalPtr(), sampler_heap_->GetInternalPtr()}.data());
  compute_pipeline_set_ = pipeline_state && pipeline_state->is_compute_;
  SetRootSignature(pipeline_state ? pipeline_state->num_params_ : 0);
  local_resource_states_.Clear();
  pending_barriers_.clear();
}


auto CommandList::End() const -> void {
  ThrowIfFailed(cmd_list_->Close(), "Failed to close command list.");
}


auto CommandList::ClearDepthStencil(Texture const& tex, D3D12_CLEAR_FLAGS const clear_flags, FLOAT const depth,
                                    UINT8 const stencil, std::span<D3D12_RECT const> const rects,
                                    UINT16 const mip_level) -> void {
  GenerateBarrier(tex, D3D12_BARRIER_SYNC_DEPTH_STENCIL, D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE,
    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE);

  cmd_list_->ClearDepthStencilView(dsv_heap_->GetDescriptorCpuHandle(tex.GetDepthStencilView(mip_level)), clear_flags,
    depth, stencil,
    static_cast<UINT>(rects.size()), rects.data());
}


auto CommandList::ClearRenderTarget(Texture const& tex, std::span<FLOAT const, 4> const color_rgba,
                                    std::span<D3D12_RECT const> const rects, UINT16 const mip_level) -> void {
  GenerateBarrier(tex, D3D12_BARRIER_SYNC_RENDER_TARGET, D3D12_BARRIER_ACCESS_RENDER_TARGET,
    D3D12_BARRIER_LAYOUT_RENDER_TARGET);

  cmd_list_->ClearRenderTargetView(rtv_heap_->GetDescriptorCpuHandle(tex.GetRenderTargetView(mip_level)),
    color_rgba.data(),
    static_cast<UINT>(rects.size()), rects.data());
}


auto CommandList::CopyBuffer(Buffer const& dst, Buffer const& src) -> void {
  GenerateBarrier(src, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_SOURCE);
  GenerateBarrier(dst, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST);
  cmd_list_->CopyResource(dst.GetInternalResource(), src.GetInternalResource());
}


auto CommandList::CopyBufferRegion(Buffer const& dst, UINT64 const dst_offset, Buffer const& src,
                                   UINT64 const src_offset, UINT64 const num_bytes) -> void {
  GenerateBarrier(src, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_SOURCE);
  GenerateBarrier(dst, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST);
  cmd_list_->CopyBufferRegion(dst.GetInternalResource(), dst_offset, src.GetInternalResource(), src_offset, num_bytes);
}


auto CommandList::CopyTexture(Texture const& dst, Texture const& src) -> void {
  GenerateBarrier(src, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_SOURCE,
    D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE);
  GenerateBarrier(dst, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST,
    D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST);
  cmd_list_->CopyResource(dst.GetInternalResource(), src.GetInternalResource());
}


auto CommandList::CopyTextureRegion(Texture const& dst, UINT const dst_subresource_index, UINT const dst_x,
                                    UINT const dst_y, UINT const dst_z, Texture const& src,
                                    UINT const src_subresource_index, D3D12_BOX const* src_box) -> void {
  GenerateBarrier(src, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_SOURCE,
    D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE);
  GenerateBarrier(dst, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST,
    D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST);

  D3D12_TEXTURE_COPY_LOCATION const dst_loc{
    .pResource = dst.GetInternalResource(), .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
    .SubresourceIndex = dst_subresource_index
  };
  D3D12_TEXTURE_COPY_LOCATION const src_loc{
    .pResource = src.GetInternalResource(), .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
    .SubresourceIndex = src_subresource_index
  };
  cmd_list_->CopyTextureRegion(&dst_loc, dst_x, dst_y, dst_z, &src_loc, src_box);
}


auto CommandList::CopyTextureRegion(Texture const& dst, UINT const dst_subresource_index, UINT const dst_x,
                                    UINT const dst_y, UINT const dst_z, Buffer const& src,
                                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& src_footprint) -> void {
  GenerateBarrier(src, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_SOURCE);
  GenerateBarrier(dst, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST,
    D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST);
  D3D12_TEXTURE_COPY_LOCATION const dst_loc{
    .pResource = dst.GetInternalResource(), .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
    .SubresourceIndex = dst_subresource_index
  };
  D3D12_TEXTURE_COPY_LOCATION const src_loc{
    .pResource = src.GetInternalResource(), .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
    .PlacedFootprint = src_footprint
  };
  cmd_list_->CopyTextureRegion(&dst_loc, dst_x, dst_y, dst_z, &src_loc, nullptr);
}


auto CommandList::DiscardRenderTarget(Texture const& tex, std::optional<D3D12_DISCARD_REGION> const& region) -> void {
  GenerateBarrier(tex, D3D12_BARRIER_SYNC_RENDER_TARGET, D3D12_BARRIER_ACCESS_RENDER_TARGET,
    D3D12_BARRIER_LAYOUT_RENDER_TARGET);
  cmd_list_->DiscardResource(tex.GetInternalResource(), region ? &*region : nullptr);
}


auto CommandList::DiscardDepthStencil(Texture const& tex, std::optional<D3D12_DISCARD_REGION> const& region) -> void {
  GenerateBarrier(tex, D3D12_BARRIER_SYNC_DEPTH_STENCIL, D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE,
    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE);
  cmd_list_->DiscardResource(tex.GetInternalResource(), region ? &*region : nullptr);
}


auto CommandList::Dispatch(UINT const thread_group_count_x, UINT const thread_group_count_y,
                           UINT const thread_group_count_z) const -> void {
  cmd_list_->Dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
}


auto CommandList::DispatchMesh(UINT const thread_group_count_x, UINT const thread_group_count_y,
                               UINT const thread_group_count_z) const -> void {
  cmd_list_->DispatchMesh(thread_group_count_x, thread_group_count_y, thread_group_count_z);
}


auto CommandList::DrawIndexedInstanced(UINT const index_count_per_instance, UINT const instance_count,
                                       UINT const start_index_location, INT const base_vertex_location,
                                       UINT const start_instance_location) const -> void {
  std::array const offsets{*std::bit_cast<UINT const*>(&base_vertex_location), start_instance_location};
  cmd_list_->SetGraphicsRoot32BitConstants(1, static_cast<UINT>(offsets.size()), offsets.data(), 0);
  cmd_list_->DrawIndexedInstanced(index_count_per_instance, instance_count, start_index_location, base_vertex_location,
    start_instance_location);
}


auto CommandList::DrawInstanced(UINT const vertex_count_per_instance, UINT const instance_count,
                                UINT const start_vertex_location, UINT const start_instance_location) const -> void {
  std::array const offsets{0u, start_instance_location};
  cmd_list_->SetGraphicsRoot32BitConstants(1, static_cast<UINT>(offsets.size()), offsets.data(), 0);
  cmd_list_->DrawInstanced(vertex_count_per_instance, instance_count, start_vertex_location, start_instance_location);
}


auto CommandList::Resolve(Texture const& dst, Texture const& src, DXGI_FORMAT const format) -> void {
  GenerateBarrier(src, D3D12_BARRIER_SYNC_RESOLVE, D3D12_BARRIER_ACCESS_RESOLVE_SOURCE,
    D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE);
  GenerateBarrier(dst, D3D12_BARRIER_SYNC_RESOLVE, D3D12_BARRIER_ACCESS_RESOLVE_DEST,
    D3D12_BARRIER_LAYOUT_RESOLVE_DEST);
  cmd_list_->ResolveSubresource(dst.GetInternalResource(), 0, src.GetInternalResource(), 0, format);
}


auto CommandList::SetBlendFactor(std::span<FLOAT const, 4> const blend_factor) const -> void {
  cmd_list_->OMSetBlendFactor(blend_factor.data());
}


auto CommandList::SetIndexBuffer(Buffer const& buf, DXGI_FORMAT const index_format) -> void {
  GenerateBarrier(buf, D3D12_BARRIER_SYNC_VERTEX_SHADING, D3D12_BARRIER_ACCESS_INDEX_BUFFER);
  D3D12_INDEX_BUFFER_VIEW const ibv{
    buf.GetInternalResource()->GetGPUVirtualAddress(), static_cast<UINT>(buf.GetInternalResource()->GetDesc1().Width),
    index_format
  };
  cmd_list_->IASetIndexBuffer(&ibv);
}


auto CommandList::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY const primitive_topology) const -> void {
  cmd_list_->IASetPrimitiveTopology(primitive_topology);
}


auto CommandList::SetRenderTargets(std::span<Texture const* const> const render_targets,
                                   Texture const* const depth_stencil, UINT16 const mip_level) -> void {
  std::ranges::for_each(render_targets, [this](Texture const* const tex) {
    if (tex) {
      GenerateBarrier(*tex, D3D12_BARRIER_SYNC_RENDER_TARGET, D3D12_BARRIER_ACCESS_RENDER_TARGET,
        D3D12_BARRIER_LAYOUT_RENDER_TARGET);
    }
  });

  if (depth_stencil) {
    GenerateBarrier(*depth_stencil, D3D12_BARRIER_SYNC_DEPTH_STENCIL,
      pipeline_allows_ds_write_ ? D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE : D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ,
      pipeline_allows_ds_write_ ? D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE : D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ);
  }

  FastVector<D3D12_CPU_DESCRIPTOR_HANDLE> rt_handles;
  rt_handles.reserve(render_targets.size());
  std::ranges::transform(render_targets, std::back_inserter(rt_handles), [this, mip_level](Texture const* const tex) {
    return tex ? rtv_heap_->GetDescriptorCpuHandle(tex->GetRenderTargetView(mip_level)) : D3D12_CPU_DESCRIPTOR_HANDLE{};
  });

  auto const ds_handle{
    depth_stencil
      ? dsv_heap_->GetDescriptorCpuHandle(depth_stencil->GetDepthStencilView(mip_level))
      : D3D12_CPU_DESCRIPTOR_HANDLE{}
  };

  cmd_list_->OMSetRenderTargets(static_cast<UINT>(rt_handles.size()), rt_handles.data(), FALSE,
    depth_stencil ? &ds_handle : nullptr);
}


auto CommandList::SetStencilRef(UINT const stencil_ref) const -> void {
  cmd_list_->OMSetStencilRef(stencil_ref);
}


auto CommandList::SetScissorRects(std::span<D3D12_RECT const> const rects) const -> void {
  cmd_list_->RSSetScissorRects(static_cast<UINT>(rects.size()), rects.data());
}


auto CommandList::SetViewports(std::span<D3D12_VIEWPORT const> const viewports) const -> void {
  cmd_list_->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
}


auto CommandList::SetPipelineParameter(UINT const index, UINT const value) const -> void {
  if (compute_pipeline_set_) {
    cmd_list_->SetComputeRoot32BitConstant(0, value, index);
  } else {
    cmd_list_->SetGraphicsRoot32BitConstant(0, value, index);
  }
}


auto CommandList::SetPipelineParameters(UINT const index, std::span<UINT const> const values) const -> void {
  if (compute_pipeline_set_) {
    cmd_list_->SetComputeRoot32BitConstants(0, static_cast<UINT>(values.size()), values.data(), index);
  } else {
    cmd_list_->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(values.size()), values.data(), index);
  }
}


auto CommandList::SetConstantBuffer(UINT const param_idx, Buffer const& buf) -> void {
  GenerateBarrier(buf, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_CONSTANT_BUFFER);
  SetPipelineParameter(param_idx, buf.GetConstantBuffer());
}


auto CommandList::SetShaderResource(UINT const param_idx, Buffer const& buf) -> void {
  GenerateBarrier(buf, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_SHADER_RESOURCE);
  SetPipelineParameter(param_idx, buf.GetShaderResource());
}


auto CommandList::SetShaderResource(UINT const param_idx, Texture const& tex) -> void {
  GenerateBarrier(tex, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_SHADER_RESOURCE,
    D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE);
  SetPipelineParameter(param_idx, tex.GetShaderResource());
}


auto CommandList::SetUnorderedAccess(UINT const param_idx, Buffer const& buf) -> void {
  GenerateBarrier(buf, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS);
  SetPipelineParameter(param_idx, buf.GetUnorderedAccess());
}


auto CommandList::SetUnorderedAccess(UINT const param_idx, Texture const& tex) -> void {
  GenerateBarrier(tex, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS,
    D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS);
  SetPipelineParameter(param_idx, tex.GetUnorderedAccess());
}


auto CommandList::SetPipelineState(PipelineState const& pipeline_state) -> void {
  cmd_list_->SetPipelineState(pipeline_state.pipeline_state_.Get());
  compute_pipeline_set_ = pipeline_state.is_compute_;
  pipeline_allows_ds_write_ = pipeline_state.allows_ds_write_;
  SetRootSignature(pipeline_state.num_params_);
}


auto CommandList::SetRootSignature(std::uint8_t const num_params) const -> void {
  if (compute_pipeline_set_) {
    cmd_list_->SetComputeRootSignature(root_signatures_->Get(num_params).Get());
  } else {
    cmd_list_->SetGraphicsRootSignature(root_signatures_->Get(num_params).Get());
  }
}


CommandList::CommandList(ComPtr<ID3D12CommandAllocator> allocator, ComPtr<ID3D12GraphicsCommandList7> cmd_list,
                         details::DescriptorHeap const* dsv_heap, details::DescriptorHeap const* rtv_heap,
                         details::DescriptorHeap const* res_desc_heap, details::DescriptorHeap const* sampler_heap,
                         details::RootSignatureCache* root_signatures) :
  allocator_{std::move(allocator)},
  cmd_list_{std::move(cmd_list)},
  dsv_heap_{dsv_heap},
  rtv_heap_{rtv_heap},
  res_desc_heap_{res_desc_heap},
  sampler_heap_{sampler_heap},
  root_signatures_{root_signatures} {}


auto CommandList::GenerateBarrier(Buffer const& buf, D3D12_BARRIER_SYNC const sync,
                                  D3D12_BARRIER_ACCESS const access) -> void {
  auto const local_state{local_resource_states_.Get(buf.GetInternalResource())};
  auto const needs_barrier{local_state && (local_state->access & access) == 0};

  if (!local_state || needs_barrier) {
    local_resource_states_.Record(buf.GetInternalResource(), {
      .sync = sync, .access = access, .layout = D3D12_BARRIER_LAYOUT_UNDEFINED
    });
  }

  if (needs_barrier) {
    D3D12_BUFFER_BARRIER const barrier{
      local_state->sync, sync, local_state->access, access, buf.GetInternalResource(), 0, UINT64_MAX
    };
    D3D12_BARRIER_GROUP const group{.Type = D3D12_BARRIER_TYPE_BUFFER, .NumBarriers = 1, .pBufferBarriers = &barrier};
    cmd_list_->Barrier(1, &group);
  }
}


auto CommandList::GenerateBarrier(Texture const& tex, D3D12_BARRIER_SYNC const sync, D3D12_BARRIER_ACCESS const access,
                                  D3D12_BARRIER_LAYOUT const layout) -> void {
  auto const local_state{local_resource_states_.Get(tex.GetInternalResource())};
  auto const needs_barrier{local_state && ((local_state->layout & layout) == 0 || (local_state->access & access) == 0)};

  if (!local_state) {
    pending_barriers_.emplace_back(layout, tex.GetInternalResource());
  }

  if (!local_state || needs_barrier) {
    local_resource_states_.Record(tex.GetInternalResource(), {.sync = sync, .access = access, .layout = layout});
  }

  if (needs_barrier) {
    D3D12_TEXTURE_BARRIER const barrier{
      local_state->sync, sync, local_state->access, access, local_state->layout, layout, tex.GetInternalResource(), {
        .IndexOrFirstMipLevel = 0xffffffff, .NumMipLevels = 0, .FirstArraySlice = 0, .NumArraySlices = 0,
        .FirstPlane = 0, .NumPlanes = 0
      },
      D3D12_TEXTURE_BARRIER_FLAG_NONE
    };
    D3D12_BARRIER_GROUP const group{.Type = D3D12_BARRIER_TYPE_TEXTURE, .NumBarriers = 1, .pTextureBarriers = &barrier};
    cmd_list_->Barrier(1, &group);
  }
}


auto Fence::GetNextValue() const -> UINT64 {
  return next_val_;
}


auto Fence::GetCompletedValue() const -> UINT64 {
  return fence_->GetCompletedValue();
}


auto Fence::Wait(UINT64 const wait_value) const -> void {
  ThrowIfFailed(fence_->SetEventOnCompletion(wait_value, nullptr), "Failed to wait fence from CPU.");
}


auto Fence::Signal() -> void {
  auto const new_fence_val{next_val_.load()};
  ThrowIfFailed(fence_->Signal(new_fence_val), "Failed to signal fence from the CPU.");
  next_val_ = new_fence_val + 1;
}


Fence::Fence(ComPtr<ID3D12Fence> fence, UINT64 const next_value) :
  fence_{std::move(fence)},
  next_val_{next_value} {}


auto SwapChain::GetTextures() const -> std::span<SharedDeviceChildHandle<Texture const> const> {
  return *std::bit_cast<std::span<SharedDeviceChildHandle<Texture const>>*>(&textures_);
}


auto SwapChain::GetCurrentTextureIndex() const -> UINT {
  return swap_chain_->GetCurrentBackBufferIndex();
}


auto SwapChain::GetCurrentTexture() const -> Texture const& {
  return *textures_[GetCurrentTextureIndex()];
}


auto SwapChain::GetSyncInterval() const -> UINT {
  return sync_interval_;
}


auto SwapChain::SetSyncInterval(UINT const sync_interval) -> void {
  sync_interval_ = sync_interval;
}


SwapChain::SwapChain(ComPtr<IDXGISwapChain4> swap_chain, UINT const present_flags) :
  swap_chain_{std::move(swap_chain)},
  present_flags_{present_flags} {}


UniqueSamplerHandle::UniqueSamplerHandle(UINT const resource, GraphicsDevice& device) :
  resource_{resource},
  device_{&device} {}


UniqueSamplerHandle::UniqueSamplerHandle(UniqueSamplerHandle&& other) noexcept :
  resource_{other.resource_},
  device_{other.device_} {
  other.resource_ = details::kInvalidResourceIndex;
  other.device_ = nullptr;
}


UniqueSamplerHandle::~UniqueSamplerHandle() {
  InternalDestruct();
}


auto UniqueSamplerHandle::operator=(UniqueSamplerHandle&& other) noexcept -> UniqueSamplerHandle& {
  if (this != &other) {
    InternalDestruct();
    resource_ = other.resource_;
    device_ = other.device_;
    other.resource_ = details::kInvalidResourceIndex;
    other.device_ = nullptr;
  }
  return *this;
}


auto UniqueSamplerHandle::Get() const -> UINT {
  return resource_;
}


auto UniqueSamplerHandle::IsValid() const -> bool {
  return resource_ != details::kInvalidResourceIndex;
}


auto UniqueSamplerHandle::InternalDestruct() const -> void {
  if (device_) {
    device_->DestroySampler(resource_);
  }
}
}
