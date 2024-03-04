#include "graphics_device.hpp"

#include <dxgidebug.h>
#include <d3dx12.h>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

using Microsoft::WRL::ComPtr;


extern "C" {
UINT const D3D12SDKVersion{D3D12_SDK_VERSION};
char const* D3D12SDKPath{".\\D3D12\\"};
}


namespace graphics {
UINT const GraphicsDevice::rtv_heap_size_{1'000'000};
UINT const GraphicsDevice::dsv_heap_size_{1'000'000};
UINT const GraphicsDevice::res_desc_heap_size_{1'000'000};
UINT const GraphicsDevice::invalid_resource_index_{static_cast<UINT>(-1)};


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
      std::move(res_desc_heap), std::move(queue)
    }
  };
}


auto GraphicsDevice::CreateBuffer(BufferDesc const& desc, D3D12_HEAP_TYPE const heap_type) -> std::unique_ptr<Buffer> {
  ComPtr<D3D12MA::Allocation> allocation;
  ComPtr<ID3D12Resource2> resource;

  D3D12MA::ALLOCATION_DESC const alloc_desc{
    D3D12MA::ALLOCATION_FLAG_NONE, heap_type, D3D12_HEAP_FLAG_NONE, nullptr, nullptr
  };

  auto const res_desc{CD3DX12_RESOURCE_DESC1::Buffer(desc.size)};

  if (FAILED(
    allocator_->CreateResource3(&alloc_desc, &res_desc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation
      , IID_PPV_ARGS(&resource)))) {
    return nullptr;
  }

  UINT cbv;

  if (desc.cbv) {
    cbv = AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CONSTANT_BUFFER_VIEW_DESC const cbv_desc{resource->GetGPUVirtualAddress(), desc.size};
    device_->CreateConstantBufferView(&cbv_desc, CD3DX12_CPU_DESCRIPTOR_HANDLE{
      res_desc_heap_start_, static_cast<INT>(cbv), res_desc_heap_increment_
    });
  } else {
    cbv = invalid_resource_index_;
  }

  UINT srv;

  if (desc.srv) {
    srv = AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_SHADER_RESOURCE_VIEW_DESC const srv_desc{
      .Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
      .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
      .Buffer = {0, desc.size / desc.stride, desc.stride, D3D12_BUFFER_SRV_FLAG_NONE}
    };
    device_->CreateShaderResourceView(resource.Get(), &srv_desc, CD3DX12_CPU_DESCRIPTOR_HANDLE{
      res_desc_heap_start_, static_cast<INT>(srv), res_desc_heap_increment_
    });
  } else {
    srv = invalid_resource_index_;
  }

  UINT uav;

  if (desc.uav) {
    uav = AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_UNORDERED_ACCESS_VIEW_DESC const uav_desc{
      .Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
      .Buffer = {
        .FirstElement = 0, .NumElements = desc.size / desc.stride, .StructureByteStride = desc.stride,
        .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
      }
    };
    device_->CreateUnorderedAccessView(resource.Get(), nullptr, &uav_desc, CD3DX12_CPU_DESCRIPTOR_HANDLE{
      res_desc_heap_start_, static_cast<INT>(uav), res_desc_heap_increment_
    });
  } else {
    uav = invalid_resource_index_;
  }

  return std::make_unique<Buffer>(std::move(allocation), std::move(resource), cbv, srv, uav);
}


auto GraphicsDevice::CreateTexture(TextureDesc const& desc, D3D12_HEAP_TYPE const heap_type,
                                   D3D12_BARRIER_LAYOUT const initial_layout,
                                   D3D12_CLEAR_VALUE const* clear_value) -> std::unique_ptr<Texture> {
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
    return nullptr;
  }

  UINT dsv;

  if (desc.dsv) {
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
    dsv = AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    device_->CreateDepthStencilView(resource.Get(), &dsv_desc,
      CD3DX12_CPU_DESCRIPTOR_HANDLE{dsv_heap_start_, static_cast<INT>(dsv), dsv_heap_increment_});
  } else {
    dsv = invalid_resource_index_;
  }

  UINT rtv;

  if (desc.rtv) {
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
    rtv = AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    device_->CreateRenderTargetView(resource.Get(), &rtv_desc,
      CD3DX12_CPU_DESCRIPTOR_HANDLE{rtv_heap_start_, static_cast<INT>(rtv), rtv_heap_increment_});
  } else {
    rtv = invalid_resource_index_;
  }

  UINT srv;

  if (desc.srv) {
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
    srv = AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device_->CreateShaderResourceView(resource.Get(), &srv_desc, CD3DX12_CPU_DESCRIPTOR_HANDLE{
      res_desc_heap_start_, static_cast<INT>(srv), res_desc_heap_increment_
    });
  } else {
    srv = invalid_resource_index_;
  }

  UINT uav;

  if (desc.uav) {
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
    uav = AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device_->CreateUnorderedAccessView(resource.Get(), nullptr, &uav_desc, CD3DX12_CPU_DESCRIPTOR_HANDLE{
      res_desc_heap_start_, static_cast<INT>(uav), res_desc_heap_increment_
    });
  } else {
    uav = invalid_resource_index_;
  }

  return std::make_unique<Texture>(std::move(allocation), std::move(resource), dsv, rtv, srv, uav);
}


auto GraphicsDevice::CreatePipelineState(PipelineStateDesc const& desc,
                                         std::uint8_t const num_32_bit_params) -> std::unique_ptr<PipelineState> {
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
        .Desc_1_1 = {1, &root_param, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED}
      };

      ComPtr<ID3DBlob> root_signature_blob;
      ComPtr<ID3DBlob> error_blob;

      if (FAILED(D3D12SerializeVersionedRootSignature(&root_signature_desc, &root_signature_blob, &error_blob))) {
        return nullptr;
      }

      if (FAILED(
        device_->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(),
          IID_PPV_ARGS(&root_signature)))) {
        return nullptr;
      }
    }
  }

  D3D12_PIPELINE_STATE_STREAM_DESC const stream_desc{sizeof(desc), &const_cast<PipelineStateDesc&>(desc)};

  ComPtr<ID3D12PipelineState> pipeline_state;

  if (FAILED(device_->CreatePipelineState(&stream_desc, IID_PPV_ARGS(&pipeline_state)))) {
    return nullptr;
  }

  return std::make_unique<PipelineState>(std::move(root_signature), std::move(pipeline_state));
}


auto GraphicsDevice::CreateCommandList() const -> std::unique_ptr<CommandList> {
  ComPtr<ID3D12CommandAllocator> allocator;
  if (FAILED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)))) {
    return nullptr;
  }

  ComPtr<ID3D12GraphicsCommandList7> cmd_list;
  if (FAILED(
    device_->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cmd_list)
    ))) {
    return nullptr;
  }

  return std::make_unique<CommandList>(std::move(allocator), std::move(cmd_list));
}


auto GraphicsDevice::CreateFence(UINT64 const initial_value) const -> ComPtr<ID3D12Fence1> {
  ComPtr<ID3D12Fence1> fence;

  if (FAILED(device_->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
    return nullptr;
  }

  return fence;
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
    return cmd_list.cmd_list.Get();
  });
  queue_->ExecuteCommandLists(static_cast<UINT>(cmd_list_submission_buffer_.size()),
    cmd_list_submission_buffer_.data());
}


GraphicsDevice::GraphicsDevice(ComPtr<IDXGIFactory7> factory, ComPtr<ID3D12Device10> device,
                               ComPtr<D3D12MA::Allocator> allocator, ComPtr<ID3D12DescriptorHeap> rtv_heap,
                               ComPtr<ID3D12DescriptorHeap> dsv_heap, ComPtr<ID3D12DescriptorHeap> res_desc_heap,
                               ComPtr<ID3D12CommandQueue> queue) :
  factory_{std::move(factory)},
  device_{std::move(device)},
  allocator_{std::move(allocator)},
  rtv_heap_{std::move(rtv_heap)},
  dsv_heap_{std::move(dsv_heap)},
  res_desc_heap_{std::move(res_desc_heap)},
  queue_{std::move(queue)},
  rtv_heap_start_{rtv_heap_->GetCPUDescriptorHandleForHeapStart()},
  dsv_heap_start_{dsv_heap_->GetCPUDescriptorHandleForHeapStart()},
  res_desc_heap_start_{res_desc_heap_->GetCPUDescriptorHandleForHeapStart()},
  rtv_heap_increment_{device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)},
  dsv_heap_increment_{device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)},
  res_desc_heap_increment_{device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)} {
  rtv_free_indices_.reserve(rtv_heap_size_);
  for (UINT i{0}; i < rtv_heap_size_; i++) {
    rtv_free_indices_.emplace_back(i);
  }

  dsv_free_indices_.reserve(dsv_heap_size_);
  for (UINT i{0}; i < dsv_heap_size_; i++) {
    dsv_free_indices_.emplace_back(i);
  }

  res_desc_free_indices_.reserve(res_desc_heap_size_);
  for (UINT i{0}; i < res_desc_heap_size_; i++) {
    res_desc_free_indices_.emplace_back(i);
  }
}


auto GraphicsDevice::AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE const type) -> UINT {
  switch (type) {
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: {
      std::scoped_lock const lock{rtv_indices_mutex_};
      auto const idx{rtv_free_indices_.back()};
      rtv_free_indices_.pop_back();
      return idx;
    }
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: {
      std::scoped_lock const lock{dsv_indices_mutex_};
      auto const idx{dsv_free_indices_.back()};
      dsv_free_indices_.pop_back();
      return idx;
    }
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: {
      std::scoped_lock const lock{res_desc_free_indices_mutex_};
      auto const idx{res_desc_free_indices_.back()};
      res_desc_free_indices_.pop_back();
      return idx;
    }
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: [[fallthrough]];
    case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES: ;
  }

  return 0;
}


auto GraphicsDevice::ReleaseDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE const type, UINT const idx) -> void {
  switch (type) {
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: {
      std::scoped_lock const lock{rtv_indices_mutex_};
      rtv_free_indices_.emplace_back(idx);
      return;
    }
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: {
      std::scoped_lock const lock{dsv_indices_mutex_};
      dsv_free_indices_.emplace_back(idx);
      return;
    }
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: {
      std::scoped_lock const lock{res_desc_free_indices_mutex_};
      res_desc_free_indices_.emplace_back(idx);
      return;
    }
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: [[fallthrough]];
    case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES: ;
  }
}
}
