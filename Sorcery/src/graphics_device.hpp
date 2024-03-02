#pragma once

#include "graphics_platform.hpp"

#include <D3D12MemAlloc.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>


extern "C" {
__declspec(dllexport) extern UINT const D3D12SDKVersion;
__declspec(dllexport) extern char const* D3D12SDKPath;
}


namespace graphics {
class GraphicsDevice {
public:
  [[nodiscard]] static auto New(bool enable_debug) -> std::unique_ptr<GraphicsDevice>;

private:
  GraphicsDevice(Microsoft::WRL::ComPtr<IDXGIFactory7> factory, Microsoft::WRL::ComPtr<ID3D12Device10> device,
                 Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap,
                 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> res_desc_heap,
                 Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue);

  [[nodiscard]] auto AllocateDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE type) -> UINT;
  auto ReleaseDescriptorIndex(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT idx) -> void;

  static UINT const rtv_heap_size_;
  static UINT const dsv_heap_size_;
  static UINT const res_desc_heap_size_;

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
};
}
