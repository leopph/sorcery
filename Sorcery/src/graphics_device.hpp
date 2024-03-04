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


namespace graphics {
struct BufferDesc {
  UINT size;
  UINT stride;
  bool cbv;
  bool srv;
  bool uav;
};


struct Buffer {
  Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation;
  Microsoft::WRL::ComPtr<ID3D12Resource2> resource;
  UINT cbv;
  UINT srv;
  UINT uav;
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


struct Texture {
  Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation;
  Microsoft::WRL::ComPtr<ID3D12Resource2> resource;
  UINT dsv;
  UINT rtv;
  UINT srv;
  UINT uav;
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


struct PipelineState {
  Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state;
  std::uint8_t num_params;
  bool is_compute;
};


struct CommandList {
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmd_list;
};


class GraphicsDevice {
public:
  [[nodiscard]] static auto New(bool enable_debug) -> std::unique_ptr<GraphicsDevice>;

  [[nodiscard]] auto CreateBuffer(BufferDesc const& desc, D3D12_HEAP_TYPE heap_type) -> std::unique_ptr<Buffer>;
  [[nodiscard]] auto CreateTexture(TextureDesc const& desc, D3D12_HEAP_TYPE heap_type,
                                   D3D12_BARRIER_LAYOUT initial_layout,
                                   D3D12_CLEAR_VALUE const* clear_value) -> std::unique_ptr<Texture>;
  [[nodiscard]] auto CreatePipelineState(PipelineStateDesc const& desc,
                                         std::uint8_t num_32_bit_params) -> std::unique_ptr<PipelineState>;
  [[nodiscard]] auto CreateCommandList() const -> std::unique_ptr<CommandList>;
  [[nodiscard]] auto CreateFence(UINT64 initial_value) const -> Microsoft::WRL::ComPtr<ID3D12Fence1>;

  [[nodiscard]] auto WaitFence(ID3D12Fence& fence, UINT64 wait_value) const -> bool;
  [[nodiscard]] auto SignalFence(ID3D12Fence& fence, UINT64 signal_value) const -> bool;
  auto ExecuteCommandLists(std::span<CommandList const> cmd_lists) -> void;

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
};
}
