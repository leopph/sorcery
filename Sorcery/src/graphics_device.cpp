#include "graphics_device.hpp"

#include "MemoryAllocation.hpp"

#include <dxgidebug.h>
#include <d3dx12.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>
#include <vector>

using Microsoft::WRL::ComPtr;


extern "C" {
UINT const D3D12SDKVersion{D3D12_SDK_VERSION};
char const* D3D12SDKPath{".\\D3D12\\"};
}


namespace sorcery::graphics {
namespace {
UINT constexpr kInvalidResourceIndex{static_cast<UINT>(-1)};
}


UINT const GraphicsDevice::rtv_heap_size_{1'000'000};
UINT const GraphicsDevice::dsv_heap_size_{1'000'000};
UINT const GraphicsDevice::res_desc_heap_size_{1'000'000};
UINT const GraphicsDevice::sampler_heap_size_{2048};


auto DescriptorHeap::Allocate() -> UINT {
  std::scoped_lock const lock{mutex_};
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
  return CD3DX12_CPU_DESCRIPTOR_HANDLE{
    heap_->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptor_index), increment_size_
  };
}


auto DescriptorHeap::GetDescriptorGpuHandle(UINT const descriptor_index) const -> D3D12_GPU_DESCRIPTOR_HANDLE {
  return CD3DX12_GPU_DESCRIPTOR_HANDLE{
    heap_->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptor_index), increment_size_
  };
}


auto DescriptorHeap::GetInternalPtr() const -> ID3D12DescriptorHeap* {
  return heap_.Get();
}


DescriptorHeap::DescriptorHeap(ComPtr<ID3D12DescriptorHeap> heap, ID3D12Device& device) :
  heap_{std::move(heap)},
  increment_size_{device.GetDescriptorHandleIncrementSize(heap_->GetDesc().Type)} {
  auto const size{heap_->GetDesc().NumDescriptors};
  free_indices_.reserve(size);
  for (UINT i{0}; i < size; i++) {
    free_indices_.emplace_back(i);
  }
}


auto GraphicsDevice::New(bool const enable_debug) -> std::unique_ptr<GraphicsDevice> {
  if (enable_debug) {
    ComPtr<ID3D12Debug6> debug;
    if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
      return nullptr;
    }

    debug->EnableDebugLayer();

    ComPtr<IDXGIInfoQueue> dxgi_info_queue;
    if FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_info_queue))) {
      return nullptr;
    }

    if (FAILED(dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE))) {
      return nullptr;
    }

    if (FAILED(
      dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE))) {
      return nullptr;
    }
  }

  UINT factory_create_flags{0};

  if (enable_debug) {
    factory_create_flags |= DXGI_CREATE_FACTORY_DEBUG;
  }

  ComPtr<IDXGIFactory7> factory;
  if (FAILED(CreateDXGIFactory2(factory_create_flags, IID_PPV_ARGS(&factory)))) {
    return nullptr;
  }

  ComPtr<IDXGIAdapter4> adapter;
  if (FAILED(factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))) {
    return nullptr;
  }

  ComPtr<ID3D12Device10> device;
  if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
    return nullptr;
  }

  if (enable_debug) {
    ComPtr<ID3D12InfoQueue> d3d12_info_queue;
    if FAILED(device.As(&d3d12_info_queue)) {
      return nullptr;
    }

    if (FAILED(d3d12_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE))) {
      return nullptr;
    }

    if (FAILED(d3d12_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE))) {
      return nullptr;
    }
  }

  CD3DX12FeatureSupport features;
  if (FAILED(features.Init(device.Get()))) {
    return nullptr;
  }

  if (features.ResourceBindingTier() < D3D12_RESOURCE_BINDING_TIER_3 || features.HighestShaderModel() <
      D3D_SHADER_MODEL_6_6 || !features.EnhancedBarriersSupported() || features.HighestRootSignatureVersion() <
      D3D_ROOT_SIGNATURE_VERSION_1_1) {
    return nullptr;
  }

  D3D12MA::ALLOCATOR_DESC const allocator_desc{D3D12MA::ALLOCATOR_FLAG_NONE, device.Get(), 0, nullptr, adapter.Get()};
  ComPtr<D3D12MA::Allocator> allocator;
  if (FAILED(CreateAllocator(&allocator_desc, &allocator))) {
    return nullptr;
  }

  D3D12_DESCRIPTOR_HEAP_DESC constexpr rtv_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtv_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0
  };
  ComPtr<ID3D12DescriptorHeap> rtv_heap;
  if (FAILED(device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)))) {
    return nullptr;
  }

  D3D12_DESCRIPTOR_HEAP_DESC constexpr dsv_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsv_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0
  };
  ComPtr<ID3D12DescriptorHeap> dsv_heap;
  if (FAILED(device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap)))) {
    return nullptr;
  }

  D3D12_DESCRIPTOR_HEAP_DESC constexpr res_desc_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, res_desc_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0
  };
  ComPtr<ID3D12DescriptorHeap> res_desc_heap;
  if (FAILED(device->CreateDescriptorHeap(&res_desc_heap_desc, IID_PPV_ARGS(&res_desc_heap)))) {
    return nullptr;
  }

  D3D12_DESCRIPTOR_HEAP_DESC constexpr sampler_heap_desc{
    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, sampler_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0
  };
  ComPtr<ID3D12DescriptorHeap> sampler_heap;
  if (FAILED(device->CreateDescriptorHeap(&sampler_heap_desc, IID_PPV_ARGS(&sampler_heap)))) {
    return nullptr;
  }

  D3D12_COMMAND_QUEUE_DESC constexpr queue_desc{
    D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE, 0
  };
  ComPtr<ID3D12CommandQueue> queue;
  if (FAILED(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&queue)))) {
    return nullptr;
  }

  return std::unique_ptr<GraphicsDevice>{
    new GraphicsDevice{
      std::move(factory), std::move(device), std::move(allocator), std::move(rtv_heap), std::move(dsv_heap),
      std::move(res_desc_heap), std::move(sampler_heap), std::move(queue)
    }
  };
}


auto GraphicsDevice::CreateBuffer(BufferDesc const& desc, D3D12_HEAP_TYPE const heap_type) -> UniqueBufferHandle {
  ComPtr<D3D12MA::Allocation> allocation;
  ComPtr<ID3D12Resource2> resource;

  D3D12MA::ALLOCATION_DESC const alloc_desc{
    D3D12MA::ALLOCATION_FLAG_NONE, heap_type, D3D12_HEAP_FLAG_NONE, nullptr, nullptr
  };

  auto const res_desc{CD3DX12_RESOURCE_DESC1::Buffer(desc.size)};

  if (FAILED(
    allocator_->CreateResource3(&alloc_desc, &res_desc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation
      , IID_PPV_ARGS(&resource)))) {
    return UniqueBufferHandle{nullptr, DeviceChildDeleter<Buffer>{*this}};
  }

  UINT cbv;

  if (desc.constant_buffer) {
    cbv = res_desc_heap_.Allocate();
    D3D12_CONSTANT_BUFFER_VIEW_DESC const cbv_desc{resource->GetGPUVirtualAddress(), desc.size};
    device_->CreateConstantBufferView(&cbv_desc, res_desc_heap_.GetDescriptorCpuHandle(cbv));
  } else {
    cbv = kInvalidResourceIndex;
  }

  UINT srv;

  if (desc.shader_resource) {
    srv = res_desc_heap_.Allocate();
    D3D12_SHADER_RESOURCE_VIEW_DESC const srv_desc{
      .Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
      .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
      .Buffer = {0, desc.size / desc.stride, desc.stride, D3D12_BUFFER_SRV_FLAG_NONE}
    };
    device_->CreateShaderResourceView(resource.Get(), &srv_desc, res_desc_heap_.GetDescriptorCpuHandle(srv));
  } else {
    srv = kInvalidResourceIndex;
  }

  UINT uav;

  if (desc.unordered_access) {
    uav = res_desc_heap_.Allocate();
    D3D12_UNORDERED_ACCESS_VIEW_DESC const uav_desc{
      .Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
      .Buffer = {
        .FirstElement = 0, .NumElements = desc.size / desc.stride, .StructureByteStride = desc.stride,
        .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
      }
    };
    device_->CreateUnorderedAccessView(resource.Get(), nullptr, &uav_desc, res_desc_heap_.GetDescriptorCpuHandle(uav));
  } else {
    uav = kInvalidResourceIndex;
  }

  return UniqueBufferHandle{
    new Buffer{std::move(allocation), std::move(resource), cbv, srv, uav}, DeviceChildDeleter<Buffer>{*this}
  };
}


auto GraphicsDevice::CreateTexture(TextureDesc const& desc, D3D12_HEAP_TYPE const heap_type,
                                   D3D12_BARRIER_LAYOUT const initial_layout,
                                   D3D12_CLEAR_VALUE const* clear_value) -> UniqueTextureHandle {
  ComPtr<D3D12MA::Allocation> allocation;
  ComPtr<ID3D12Resource2> resource;

  D3D12_RESOURCE_DESC1 res_desc;

  switch (desc.dimension) {
    case TextureDimension::k1D: {
      res_desc = CD3DX12_RESOURCE_DESC1::Tex1D(desc.format, desc.width, desc.depth_or_array_size, desc.mip_levels,
        desc.flags);
      break;
    }
    case TextureDimension::k2D: {
      res_desc = CD3DX12_RESOURCE_DESC1::Tex2D(desc.format, desc.width, desc.height, desc.depth_or_array_size,
        desc.mip_levels, desc.sample_desc.Count, desc.sample_desc.Quality, desc.flags);
      break;
    }
    case TextureDimension::k3D: [[fallthrough]];
    case TextureDimension::kCube: {
      res_desc = CD3DX12_RESOURCE_DESC1::Tex3D(desc.format, desc.width, desc.height, desc.depth_or_array_size,
        desc.mip_levels, desc.flags);
      break;
    }
  }

  D3D12MA::ALLOCATION_DESC const alloc_desc{
    D3D12MA::ALLOCATION_FLAG_NONE, heap_type, D3D12_HEAP_FLAG_NONE, nullptr, nullptr
  };

  if (FAILED(
    allocator_->CreateResource3(&alloc_desc, &res_desc, initial_layout, clear_value, 0, nullptr, &allocation,
      IID_PPV_ARGS(&resource)))) {
    return UniqueTextureHandle{nullptr, DeviceChildDeleter<Texture>{*this}};
  }

  UINT dsv;

  if (desc.depth_stencil) {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{.Format = desc.format, .Flags = D3D12_DSV_FLAG_NONE};
    if (desc.dimension == TextureDimension::k1D) {
      if (desc.depth_or_array_size == 1) {
        dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
        dsv_desc.Texture1D.MipSlice = 0;
      } else {
        dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
        dsv_desc.Texture1DArray.MipSlice = 0;
        dsv_desc.Texture1DArray.FirstArraySlice = 0;
        dsv_desc.Texture1DArray.ArraySize = desc.depth_or_array_size;
      }
    } else if (desc.dimension == TextureDimension::k2D) {
      if (desc.depth_or_array_size == 1) {
        if (desc.sample_desc.Count == 1) {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
          dsv_desc.Texture2D.MipSlice = 0;
        } else {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        }
      } else {
        if (desc.sample_desc.Count == 1) {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
          dsv_desc.Texture2DArray.MipSlice = 0;
          dsv_desc.Texture2DArray.FirstArraySlice = 0;
          dsv_desc.Texture2DArray.ArraySize = desc.depth_or_array_size;
        } else {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
          dsv_desc.Texture2DMSArray.FirstArraySlice = 0;
          dsv_desc.Texture2DMSArray.ArraySize = desc.depth_or_array_size;
        }
      }
    }
    dsv = dsv_heap_.Allocate();
    device_->CreateDepthStencilView(resource.Get(), &dsv_desc, dsv_heap_.GetDescriptorCpuHandle(dsv));
  } else {
    dsv = kInvalidResourceIndex;
  }

  UINT rtv;

  if (desc.render_target) {
    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{.Format = desc.format};
    if (desc.dimension == TextureDimension::k1D) {
      if (desc.depth_or_array_size == 1) {
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
        rtv_desc.Texture1D.MipSlice = 0;
      } else {
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
        rtv_desc.Texture1DArray.MipSlice = 0;
        rtv_desc.Texture1DArray.FirstArraySlice = 0;
        rtv_desc.Texture1DArray.ArraySize = desc.depth_or_array_size;
      }
    } else if (desc.dimension == TextureDimension::k2D) {
      if (desc.depth_or_array_size == 1) {
        if (desc.sample_desc.Count == 1) {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
          rtv_desc.Texture2D.MipSlice = 0;
          rtv_desc.Texture2D.PlaneSlice = 0;
        } else {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        }
      } else {
        if (desc.sample_desc.Count == 1) {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
          rtv_desc.Texture2DArray.MipSlice = 0;
          rtv_desc.Texture2DArray.FirstArraySlice = 0;
          rtv_desc.Texture2DArray.ArraySize = desc.depth_or_array_size;
          rtv_desc.Texture2DArray.PlaneSlice = 0;
        } else {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
          rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
          rtv_desc.Texture2DMSArray.ArraySize = desc.depth_or_array_size;
        }
      }
    } else if (desc.dimension == TextureDimension::k3D || desc.dimension == TextureDimension::kCube) {
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
      rtv_desc.Texture3D.MipSlice = 0;
      rtv_desc.Texture3D.FirstWSlice = 0;
      rtv_desc.Texture3D.WSize = static_cast<UINT>(-1);
    }
    rtv = rtv_heap_.Allocate();
    device_->CreateRenderTargetView(resource.Get(), &rtv_desc, rtv_heap_.GetDescriptorCpuHandle(rtv));
  } else {
    rtv = kInvalidResourceIndex;
  }

  UINT srv;

  if (desc.shader_resource) {
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{
      .Format = desc.format, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
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
        if (desc.sample_desc.Count == 1) {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
          srv_desc.Texture2D.MostDetailedMip = 0;
          srv_desc.Texture2D.MipLevels = static_cast<UINT>(-1);
          srv_desc.Texture2D.PlaneSlice = 0;
          srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
        } else {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
      } else {
        if (desc.sample_desc.Count == 1) {
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
      srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      srv_desc.TextureCube.MostDetailedMip = 0;
      srv_desc.TextureCube.MipLevels = static_cast<UINT>(-1);
      srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
    }
    srv = res_desc_heap_.Allocate();
    device_->CreateShaderResourceView(resource.Get(), &srv_desc, res_desc_heap_.GetDescriptorCpuHandle(srv));
  } else {
    srv = kInvalidResourceIndex;
  }

  UINT uav;

  if (desc.unordered_access) {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{.Format = desc.format};
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
    } else if (desc.dimension == TextureDimension::k2D) {
      if (desc.depth_or_array_size == 1) {
        if (desc.sample_desc.Count == 1) {
          uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
          uav_desc.Texture2D.MipSlice = 0;
          uav_desc.Texture2D.PlaneSlice = 0;
        } else {
          uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMS;
        }
      } else {
        if (desc.sample_desc.Count == 1) {
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
      }
    } else if (desc.dimension == TextureDimension::k3D || desc.dimension == TextureDimension::kCube) {
      uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
      uav_desc.Texture3D.MipSlice = 0;
      uav_desc.Texture3D.FirstWSlice = 0;
      uav_desc.Texture3D.WSize = static_cast<UINT>(-1);
    }
    uav = res_desc_heap_.Allocate();
    device_->CreateUnorderedAccessView(resource.Get(), nullptr, &uav_desc, res_desc_heap_.GetDescriptorCpuHandle(uav));
  } else {
    uav = kInvalidResourceIndex;
  }

  return UniqueTextureHandle{
    new Texture{std::move(allocation), std::move(resource), dsv, rtv, srv, uav}, DeviceChildDeleter<Texture>{*this}
  };
}


auto GraphicsDevice::CreatePipelineState(PipelineStateDesc const& desc,
                                         std::uint8_t const num_32_bit_params) -> UniquePipelineStateHandle {
  ComPtr<ID3D12RootSignature> root_signature;

  {
    std::scoped_lock const lock{root_signature_mutex_};

    if (auto const it{root_signatures_.find(num_32_bit_params)}; it != std::end(root_signatures_)) {
      root_signature = it->second;
    } else {
      CD3DX12_ROOT_PARAMETER1 root_param;
      root_param.InitAsConstants(num_32_bit_params, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

      D3D12_VERSIONED_ROOT_SIGNATURE_DESC const root_signature_desc{
        .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
        .Desc_1_1 = {
          1, &root_param, 0, nullptr,
          D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
          D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
        }
      };

      ComPtr<ID3DBlob> root_signature_blob;
      ComPtr<ID3DBlob> error_blob;

      if (FAILED(D3D12SerializeVersionedRootSignature(&root_signature_desc, &root_signature_blob, &error_blob))) {
        return UniquePipelineStateHandle{nullptr, DeviceChildDeleter<PipelineState>{*this}};
      }

      if (FAILED(
        device_->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(),
          IID_PPV_ARGS(&root_signature)))) {
        return UniquePipelineStateHandle{nullptr, DeviceChildDeleter<PipelineState>{*this}};
      }

      root_signatures_.emplace(num_32_bit_params, root_signature);
    }
  }

  D3D12_PIPELINE_STATE_STREAM_DESC const stream_desc{sizeof(desc), &const_cast<PipelineStateDesc&>(desc)};

  ComPtr<ID3D12PipelineState> pipeline_state;

  if (FAILED(device_->CreatePipelineState(&stream_desc, IID_PPV_ARGS(&pipeline_state)))) {
    return UniquePipelineStateHandle{nullptr, DeviceChildDeleter<PipelineState>{*this}};
  }

  return UniquePipelineStateHandle{
    new PipelineState{
      std::move(root_signature), std::move(pipeline_state), num_32_bit_params,
      static_cast<D3D12_SHADER_BYTECODE>(desc.vs).BytecodeLength != 0
    },
    DeviceChildDeleter<PipelineState>{*this}
  };
}


auto GraphicsDevice::CreateCommandList() -> UniqueCommandListHandle {
  ComPtr<ID3D12CommandAllocator> allocator;
  if (FAILED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)))) {
    return UniqueCommandListHandle{nullptr, DeviceChildDeleter<CommandList>{*this}};
  }

  ComPtr<ID3D12GraphicsCommandList7> cmd_list;
  if (FAILED(
    device_->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cmd_list)
    ))) {
    return UniqueCommandListHandle{nullptr, DeviceChildDeleter<CommandList>{*this}};
  }

  return UniqueCommandListHandle{
    new CommandList{std::move(allocator), std::move(cmd_list)}, DeviceChildDeleter<CommandList>{*this}
  };
}


auto GraphicsDevice::CreateFence(UINT64 const initial_value) -> ComPtr<ID3D12Fence1> {
  ComPtr<ID3D12Fence1> fence;

  if (FAILED(device_->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
    return nullptr;
  }

  return fence;
}


auto GraphicsDevice::CreateSwapChain(SwapChainDesc const& desc, HWND const window_handle) -> UniqueSwapChainHandle {
  DXGI_SWAP_CHAIN_DESC1 const dxgi_desc{
    desc.width, desc.height, DXGI_FORMAT_R8G8B8A8_UNORM, FALSE, {1, 0}, desc.usage, desc.buffer_count, desc.scaling,
    DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_UNSPECIFIED, swap_chain_flags_
  };

  ComPtr<IDXGISwapChain1> swap_chain1;
  if (FAILED(
    factory_->CreateSwapChainForHwnd(queue_.Get(), window_handle, &dxgi_desc, nullptr, nullptr, &swap_chain1))) {
    return UniqueSwapChainHandle{nullptr, DeviceChildDeleter<SwapChain>{*this}};
  }

  ComPtr<IDXGISwapChain4> swap_chain4;
  if (FAILED(swap_chain1.As(&swap_chain4))) {
    return UniqueSwapChainHandle{nullptr, DeviceChildDeleter<SwapChain>{*this}};
  }

  auto const swap_chain{new SwapChain{std::move(swap_chain4)}};
  SwapChainCreateTextures(*swap_chain);

  return UniqueSwapChainHandle{swap_chain, DeviceChildDeleter<SwapChain>{*this}};
}


auto GraphicsDevice::CreateSampler(D3D12_SAMPLER_DESC const& desc) -> UniqueSamplerHandle {
  auto const sampler{sampler_heap_.Allocate()};
  device_->CreateSampler(&desc, sampler_heap_.GetDescriptorCpuHandle(sampler));
  return UniqueSamplerHandle{sampler, *this};
}


auto GraphicsDevice::DestroyBuffer(Buffer const* const buffer) -> void {
  if (buffer) {
    res_desc_heap_.Release(buffer->cbv_);
    res_desc_heap_.Release(buffer->srv_);
    res_desc_heap_.Release(buffer->uav_);

    delete buffer;
  }
}


auto GraphicsDevice::DestroyTexture(Texture const* const texture) -> void {
  if (texture) {
    dsv_heap_.Release(texture->dsv_);
    rtv_heap_.Release(texture->rtv_);
    res_desc_heap_.Release(texture->srv_);
    res_desc_heap_.Release(texture->uav_);

    delete texture;
  }
}


auto GraphicsDevice::DestroyPipelineState(PipelineState const* const pipeline_state) -> void {
  delete pipeline_state;
}


auto GraphicsDevice::DestroyCommandList(CommandList const* const command_list) -> void {
  delete command_list;
}


auto GraphicsDevice::DestroySwapChain(SwapChain const* const swap_chain) -> void {
  delete swap_chain;
}


auto GraphicsDevice::DestroySampler(UINT const sampler) -> void {
  sampler_heap_.Release(sampler);
}


auto GraphicsDevice::WaitFence(ID3D12Fence& fence, UINT64 const wait_value) const -> bool {
  return SUCCEEDED(queue_->Wait(&fence, wait_value));
}


auto GraphicsDevice::SignalFence(ID3D12Fence& fence, UINT64 const signal_value) const -> bool {
  return SUCCEEDED(queue_->Signal(&fence, signal_value));
}


auto GraphicsDevice::ExecuteCommandLists(std::span<CommandList const> const cmd_lists) -> void {
  std::scoped_lock const lock{cmd_list_submission_mutex_};
  cmd_list_submission_buffer_.reserve(cmd_lists.size());
  cmd_list_submission_buffer_.clear();
  std::ranges::transform(cmd_lists, std::back_inserter(cmd_list_submission_buffer_), [](CommandList const& cmd_list) {
    return cmd_list.cmd_list_.Get();
  });
  queue_->ExecuteCommandLists(static_cast<UINT>(cmd_list_submission_buffer_.size()),
    cmd_list_submission_buffer_.data());
}


auto GraphicsDevice::CmdBegin(CommandList& cmd_list, PipelineState const& pipeline_state) const -> bool {
  if (FAILED(cmd_list.allocator_->Reset()) || FAILED(
        cmd_list.cmd_list_->Reset(cmd_list.allocator_.Get(), pipeline_state.pipeline_state_.Get()))) {
    return false;
  }

  cmd_list.cmd_list_->SetDescriptorHeaps(2,
    std::array{res_desc_heap_.GetInternalPtr(), sampler_heap_.GetInternalPtr()}.data());
  cmd_list.compute_pipeline_set_ = pipeline_state.is_compute_;
  SetRootSignature(cmd_list, pipeline_state.num_params_);
  return true;
}


auto GraphicsDevice::CmdEnd(CommandList const& cmd_list) const -> bool {
  return SUCCEEDED(cmd_list.cmd_list_->Close());
}


auto GraphicsDevice::CmdBarrier(CommandList const& cmd_list, std::span<GlobalBarrier const> const global_barriers,
                                std::span<BufferBarrier const> const buffer_barriers,
                                std::span<TextureBarrier const> const texture_barriers) const -> void {
  std::pmr::vector<D3D12_GLOBAL_BARRIER> globals{&GetTmpMemRes()};
  globals.reserve(global_barriers.size());
  std::pmr::vector<D3D12_BUFFER_BARRIER> buffers{&GetTmpMemRes()};
  buffers.reserve(buffer_barriers.size());
  std::pmr::vector<D3D12_TEXTURE_BARRIER> textures{&GetTmpMemRes()};
  textures.reserve(texture_barriers.size());
  std::pmr::vector<D3D12_BARRIER_GROUP> groups{&GetTmpMemRes()};

  if (!global_barriers.empty()) {
    std::ranges::transform(global_barriers, std::back_inserter(globals), [](GlobalBarrier const& barrier) {
      return D3D12_GLOBAL_BARRIER{barrier.sync_before, barrier.sync_after, barrier.access_before, barrier.access_after};
    });
    groups.emplace_back(D3D12_BARRIER_GROUP{
      .Type = D3D12_BARRIER_TYPE_GLOBAL, .NumBarriers = static_cast<UINT32>(globals.size()),
      .pGlobalBarriers = globals.data()
    });
  }

  if (!buffers.empty()) {
    std::ranges::transform(buffer_barriers, std::back_inserter(buffers), [](BufferBarrier const& barrier) {
      return D3D12_BUFFER_BARRIER{
        barrier.sync_before, barrier.sync_after, barrier.access_before, barrier.access_after,
        barrier.buffer->resource_.Get(), barrier.offset, barrier.size
      };
    });
    groups.emplace_back(D3D12_BARRIER_GROUP{
      .Type = D3D12_BARRIER_TYPE_BUFFER, .NumBarriers = static_cast<UINT32>(buffers.size()),
      .pBufferBarriers = buffers.data()
    });
  }

  if (!textures.empty()) {
    std::ranges::transform(texture_barriers, std::back_inserter(textures), [](TextureBarrier const& barrier) {
      return D3D12_TEXTURE_BARRIER{
        barrier.sync_before, barrier.sync_after, barrier.access_before, barrier.access_after, barrier.layout_before,
        barrier.layout_after, barrier.texture->resource_.Get(), barrier.subresources, barrier.flags
      };
    });
    groups.emplace_back(D3D12_BARRIER_GROUP{
      .Type = D3D12_BARRIER_TYPE_TEXTURE, .NumBarriers = static_cast<UINT32>(textures.size()),
      .pTextureBarriers = textures.data()
    });
  }

  cmd_list.cmd_list_->Barrier(static_cast<UINT32>(groups.size()), groups.data());
}


auto GraphicsDevice::CmdClearDepthStencil(CommandList const& cmd_list, Texture const& tex,
                                          D3D12_CLEAR_FLAGS const clear_flags, FLOAT const depth, UINT8 const stencil,
                                          std::span<D3D12_RECT const> const rects) const -> void {
  if (tex.dsv_ != kInvalidResourceIndex) {
    cmd_list.cmd_list_->ClearDepthStencilView(dsv_heap_.GetDescriptorCpuHandle(tex.dsv_), clear_flags, depth, stencil,
      static_cast<UINT>(rects.size()), rects.data());
  }
}


auto GraphicsDevice::CmdClearRenderTarget(CommandList const& cmd_list, Texture const& tex,
                                          std::span<FLOAT const, 4> const color_rgba,
                                          std::span<D3D12_RECT const> const rects) const -> void {
  if (tex.rtv_ != kInvalidResourceIndex) {
    cmd_list.cmd_list_->ClearRenderTargetView(rtv_heap_.GetDescriptorCpuHandle(tex.rtv_), color_rgba.data(),
      static_cast<UINT>(rects.size()), rects.data());
  }
}


auto GraphicsDevice::CmdCopyBuffer(CommandList const& cmd_list, Buffer const& dst, Buffer const& src) const -> void {
  cmd_list.cmd_list_->CopyResource(dst.resource_.Get(), src.resource_.Get());
}


auto GraphicsDevice::CmdCopyBufferRegion(CommandList const& cmd_list, Buffer const& dst, UINT64 const dst_offset,
                                         Buffer const& src, UINT64 const src_offset,
                                         UINT64 const num_bytes) const -> void {
  cmd_list.cmd_list_->CopyBufferRegion(dst.resource_.Get(), dst_offset, src.resource_.Get(), src_offset, num_bytes);
}


auto GraphicsDevice::CmdCopyTexture(CommandList const& cmd_list, Texture const& dst, Texture const& src) -> void {
  cmd_list.cmd_list_->CopyResource(dst.resource_.Get(), src.resource_.Get());
}


auto GraphicsDevice::CmdCopyTextureRegion(CommandList const& cmd_list, Texture const& dst,
                                          UINT const dst_subresource_index, UINT const dst_x, UINT const dst_y,
                                          UINT const dst_z, Texture const& src, UINT const src_subresource_index,
                                          D3D12_BOX const* src_box) const -> void {
  D3D12_TEXTURE_COPY_LOCATION const dst_loc{
    .pResource = dst.resource_.Get(), .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
    .SubresourceIndex = dst_subresource_index
  };
  D3D12_TEXTURE_COPY_LOCATION const src_loc{
    .pResource = src.resource_.Get(), .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
    .SubresourceIndex = src_subresource_index
  };
  cmd_list.cmd_list_->CopyTextureRegion(&dst_loc, dst_x, dst_y, dst_z, &src_loc, src_box);
}


auto GraphicsDevice::CmdCopyTextureRegion(CommandList const& cmd_list, Texture const& dst,
                                          UINT const dst_subresource_index, UINT const dst_x, UINT const dst_y,
                                          UINT const dst_z, Buffer const& src,
                                          D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& src_footprint) const -> void {
  D3D12_TEXTURE_COPY_LOCATION const dst_loc{
    .pResource = dst.resource_.Get(), .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
    .SubresourceIndex = dst_subresource_index
  };
  D3D12_TEXTURE_COPY_LOCATION const src_loc{
    .pResource = src.resource_.Get(), .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, .PlacedFootprint = src_footprint
  };
  cmd_list.cmd_list_->CopyTextureRegion(&dst_loc, dst_x, dst_y, dst_z, &src_loc, nullptr);
}


auto GraphicsDevice::CmdDispatch(CommandList const& cmd_list, UINT const thread_group_count_x,
                                 UINT const thread_group_count_y, UINT const thread_group_count_z) const -> void {
  cmd_list.cmd_list_->Dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
}


auto GraphicsDevice::CmdDispatchMesh(CommandList const& cmd_list, UINT const thread_group_count_x,
                                     UINT const thread_group_count_y, UINT const thread_group_count_z) const -> void {
  cmd_list.cmd_list_->DispatchMesh(thread_group_count_x, thread_group_count_y, thread_group_count_z);
}


auto GraphicsDevice::CmdDrawIndexedInstanced(CommandList const& cmd_list, UINT const index_count_per_instance,
                                             UINT const instance_count, UINT const start_index_location,
                                             INT const base_vertex_location,
                                             UINT const start_instance_location) const -> void {
  cmd_list.cmd_list_->DrawIndexedInstanced(index_count_per_instance, instance_count, start_index_location,
    base_vertex_location, start_instance_location);
}


auto GraphicsDevice::CmdDrawInstanced(CommandList const& cmd_list, UINT const vertex_count_per_instance,
                                      UINT const instance_count, UINT const start_vertex_location,
                                      UINT const start_instance_location) const -> void {
  cmd_list.cmd_list_->DrawInstanced(vertex_count_per_instance, instance_count, start_vertex_location,
    start_instance_location);
}


auto GraphicsDevice::CmdSetBlendFactor(CommandList const& cmd_list,
                                       std::span<FLOAT const, 4> const blend_factor) const -> void {
  cmd_list.cmd_list_->OMSetBlendFactor(blend_factor.data());
}


auto GraphicsDevice::CmdSetRenderTargets(CommandList const& cmd_list, std::span<Texture const> render_targets,
                                         Texture const* depth_stencil) const -> void {
  std::pmr::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rt{&GetTmpMemRes()};
  rt.reserve(render_targets.size());
  std::ranges::transform(render_targets, std::back_inserter(rt), [this](Texture const& tex) {
    return rtv_heap_.GetDescriptorCpuHandle(tex.rtv_);
  });

  auto const ds{dsv_heap_.GetDescriptorCpuHandle(depth_stencil ? depth_stencil->dsv_ : 0)};

  cmd_list.cmd_list_->OMSetRenderTargets(static_cast<UINT>(rt.size()), rt.data(), FALSE, depth_stencil ? &ds : nullptr);
}


auto GraphicsDevice::CmdSetStencilRef(CommandList const& cmd_list, UINT const stencil_ref) const -> void {
  cmd_list.cmd_list_->OMSetStencilRef(stencil_ref);
}


auto GraphicsDevice::CmdSetScissorRects(CommandList const& cmd_list,
                                        std::span<D3D12_RECT const> const rects) const -> void {
  cmd_list.cmd_list_->RSSetScissorRects(static_cast<UINT>(rects.size()), rects.data());
}


auto GraphicsDevice::CmdSetViewports(CommandList const& cmd_list,
                                     std::span<D3D12_VIEWPORT const> const viewports) const -> void {
  cmd_list.cmd_list_->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
}


auto GraphicsDevice::CmdSetPipelineParameter(CommandList const& cmd_list, UINT const index,
                                             UINT const value) const -> void {
  if (cmd_list.compute_pipeline_set_) {
    cmd_list.cmd_list_->SetComputeRoot32BitConstant(0, value, index);
  } else {
    cmd_list.cmd_list_->SetGraphicsRoot32BitConstant(0, value, index);
  }
}


auto GraphicsDevice::CmdSetPipelineParameters(CommandList const& cmd_list, UINT const index,
                                              std::span<UINT const> const values) const -> void {
  if (cmd_list.compute_pipeline_set_) {
    cmd_list.cmd_list_->SetComputeRoot32BitConstants(0, static_cast<UINT>(values.size()), values.data(), index);
  } else {
    cmd_list.cmd_list_->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(values.size()), values.data(), index);
  }
}


auto GraphicsDevice::CmdSetPipelineState(CommandList& cmd_list, PipelineState const& pipeline_state) const -> void {
  cmd_list.cmd_list_->SetPipelineState(pipeline_state.pipeline_state_.Get());
  cmd_list.compute_pipeline_set_ = pipeline_state.is_compute_;
  SetRootSignature(cmd_list, pipeline_state.num_params_);
}


auto GraphicsDevice::CmdSetStreamOutputTargets(CommandList const& cmd_list, UINT const start_slot,
                                               std::span<D3D12_STREAM_OUTPUT_BUFFER_VIEW const> const views) const ->
  void {
  cmd_list.cmd_list_->SOSetTargets(start_slot, static_cast<UINT>(views.size()), views.data());
}


auto GraphicsDevice::SwapChainGetBuffers(SwapChain const& swap_chain) const -> std::span<UniqueTextureHandle const> {
  return swap_chain.textures_;
}


auto GraphicsDevice::SwapChainGetCurrentBufferIndex(SwapChain const& swap_chain) const -> UINT {
  return swap_chain.swap_chain_->GetCurrentBackBufferIndex();
}


auto GraphicsDevice::SwapChainPresent(SwapChain const& swap_chain, UINT const sync_interval) const -> bool {
  return SUCCEEDED(swap_chain.swap_chain_->Present(sync_interval, present_flags_));
}


auto GraphicsDevice::SwapChainResize(SwapChain& swap_chain, UINT const width, UINT const height) -> bool {
  swap_chain.textures_.clear();

  if (FAILED(swap_chain.swap_chain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, swap_chain_flags_))) {
    return false;
  }

  return SwapChainCreateTextures(swap_chain);
}


GraphicsDevice::GraphicsDevice(ComPtr<IDXGIFactory7> factory, ComPtr<ID3D12Device10> device,
                               ComPtr<D3D12MA::Allocator> allocator, ComPtr<ID3D12DescriptorHeap> rtv_heap,
                               ComPtr<ID3D12DescriptorHeap> dsv_heap, ComPtr<ID3D12DescriptorHeap> res_desc_heap,
                               ComPtr<ID3D12DescriptorHeap> sampler_heap, ComPtr<ID3D12CommandQueue> queue) :
  factory_{std::move(factory)},
  device_{std::move(device)},
  allocator_{std::move(allocator)},
  rtv_heap_{std::move(rtv_heap), *device_.Get()},
  dsv_heap_{std::move(dsv_heap), *device_.Get()},
  res_desc_heap_{std::move(res_desc_heap), *device_.Get()},
  sampler_heap_{std::move(sampler_heap), *device_.Get()},
  queue_{std::move(queue)} {
  if (BOOL allow_tearing; SUCCEEDED(
    factory_->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing)
    )) && allow_tearing) {
    swap_chain_flags_ |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    present_flags_ |= DXGI_PRESENT_ALLOW_TEARING;
  }
}


auto GraphicsDevice::SetRootSignature(CommandList const& cmd_list, std::uint8_t const num_params) const -> void {
  if (cmd_list.compute_pipeline_set_) {
    cmd_list.cmd_list_->SetComputeRootSignature(root_signatures_.at(num_params).Get());
  } else {
    cmd_list.cmd_list_->SetGraphicsRootSignature(root_signatures_.at(num_params).Get());
  }
}


auto GraphicsDevice::SwapChainCreateTextures(SwapChain& swap_chain) -> bool {
  DXGI_SWAP_CHAIN_DESC1 desc;
  if (FAILED(swap_chain.swap_chain_->GetDesc1(&desc))) {
    return false;
  }

  for (UINT i{0}; i < desc.BufferCount; i++) {
    ComPtr<ID3D12Resource2> buf;
    if (FAILED(swap_chain.swap_chain_->GetBuffer(i, IID_PPV_ARGS(&buf)))) {
      return false;
    }

    UINT const rtv{desc.BufferUsage & DXGI_USAGE_RENDER_TARGET_OUTPUT ? rtv_heap_.Allocate() : kInvalidResourceIndex};
    UINT const srv{desc.BufferUsage & DXGI_USAGE_SHADER_INPUT ? res_desc_heap_.Allocate() : kInvalidResourceIndex};

    swap_chain.textures_.emplace_back(new Texture{
      nullptr, std::move(buf), kInvalidResourceIndex, rtv, srv, kInvalidResourceIndex
    }, DeviceChildDeleter<Texture>{*this});
  }

  return true;
}


auto DeviceChildDeleter<Buffer>::operator()(Buffer const* const device_child) const -> void {
  device_->DestroyBuffer(device_child);
}


auto DeviceChildDeleter<Texture>::operator()(Texture const* const device_child) const -> void {
  device_->DestroyTexture(device_child);
}


auto Buffer::GetConstantBuffer() const -> UINT {
  return cbv_;
}


auto Buffer::GetShaderResource() const -> UINT {
  return srv_;
}


auto Buffer::GetUnorderedAccess() const -> UINT {
  return uav_;
}


Buffer::Buffer(ComPtr<D3D12MA::Allocation> allocation, ComPtr<ID3D12Resource2> resource, UINT const cbv, UINT const srv,
               UINT const uav) :
  allocation_{std::move(allocation)},
  resource_{std::move(resource)},
  cbv_{cbv},
  srv_{srv},
  uav_{uav} {}


auto Texture::GetShaderResource() const -> UINT {
  return srv_;
}


auto Texture::GetUnorderedAccess() const -> UINT {
  return uav_;
}


Texture::Texture(ComPtr<D3D12MA::Allocation> allocation, ComPtr<ID3D12Resource2> resource, UINT const dsv,
                 UINT const rtv, UINT const srv, UINT const uav) :
  allocation_{std::move(allocation)},
  resource_{std::move(resource)},
  dsv_{dsv},
  rtv_{rtv},
  srv_{srv},
  uav_{uav} {}


PipelineState::PipelineState(ComPtr<ID3D12RootSignature> root_signature, ComPtr<ID3D12PipelineState> pipeline_state,
                             std::uint8_t const num_params, bool const is_compute) :
  root_signature_{std::move(root_signature)},
  pipeline_state_{std::move(pipeline_state)},
  num_params_{num_params},
  is_compute_{is_compute} {}


CommandList::CommandList(ComPtr<ID3D12CommandAllocator> allocator, ComPtr<ID3D12GraphicsCommandList7> cmd_list) :
  allocator_{std::move(allocator)},
  cmd_list_{std::move(cmd_list)} {}


auto DeviceChildDeleter<PipelineState>::operator()(PipelineState const* const device_child) const -> void {
  device_->DestroyPipelineState(device_child);
}


auto DeviceChildDeleter<CommandList>::operator()(CommandList const* const device_child) const -> void {
  device_->DestroyCommandList(device_child);
}


SwapChain::SwapChain(ComPtr<IDXGISwapChain4> swap_chain) :
  swap_chain_{std::move(swap_chain)} {}


auto DeviceChildDeleter<SwapChain>::operator()(SwapChain const* const device_child) const -> void {
  device_->DestroySwapChain(device_child);
}


auto UniqueSamplerHandle::Get() const -> UINT {
  return sampler_;
}


UniqueSamplerHandle::UniqueSamplerHandle(UINT const sampler, GraphicsDevice& device) :
  device_{&device},
  sampler_{sampler} {}


UniqueSamplerHandle::UniqueSamplerHandle(UniqueSamplerHandle&& other) noexcept :
  device_{other.device_},
  sampler_{other.sampler_} {
  other.sampler_ = kInvalidResourceIndex;
}


UniqueSamplerHandle::~UniqueSamplerHandle() {
  device_->DestroySampler(sampler_);
}


auto UniqueSamplerHandle::operator=(UniqueSamplerHandle&& other) noexcept -> UniqueSamplerHandle& {
  if (this != &other) {
    device_->DestroySampler(sampler_);
    device_ = other.device_;
    sampler_ = other.sampler_;
    other.sampler_ = kInvalidResourceIndex;
  }
  return *this;
}
}
