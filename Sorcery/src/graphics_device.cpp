#include "graphics_device.hpp"

#include <dxgidebug.h>
#include <d3dx12.h>

#include <utility>

using Microsoft::WRL::ComPtr;


extern "C" {
UINT const D3D12SDKVersion{D3D12_SDK_VERSION};
char const* D3D12SDKPath{".\\D3D12\\"};
}


namespace graphics {
UINT const GraphicsDevice::rtv_heap_size_{1'000'000};
UINT const GraphicsDevice::dsv_heap_size_{1'000'000};
UINT const GraphicsDevice::res_desc_heap_size_{1'000'000};


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
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, res_desc_heap_size_, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0
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
  queue_{std::move(queue)} {
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
