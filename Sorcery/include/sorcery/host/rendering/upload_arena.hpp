#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include <wand/wand.hpp>

#include <sorcery/Core.hpp>
#include <sorcery/observer_ptr.hpp>


namespace sorcery::rendering {
class UploadArena {
public:
  explicit UploadArena(wand::GraphicsDevice& device);

  SORCERYAPI
  auto UploadBuffer(
    wand::CommandList& cmd,
    wand::SharedDeviceHandle<wand::Buffer> const& buf,
    UINT64 byte_offset,
    std::span<std::byte const> data
  ) -> void;

  SORCERYAPI
  auto UploadBuffer(
    wand::CommandList& cmd,
    wand::SharedDeviceHandle<wand::BufferView> const& view,
    UINT64 byte_offset,
    std::span<std::byte const> data
  ) -> void;

  SORCERYAPI
  auto UploadTexture(
    wand::CommandList& cmd,
    wand::SharedDeviceHandle<wand::Texture> const& tex,
    UINT first_subresource,
    std::span<D3D12_SUBRESOURCE_DATA const> data
  ) -> void;

  auto Reset() -> void;

private:
  struct UploadAllocation {
    std::size_t page_idx;
    UINT64 offset;
  };


  struct UploadBufferPage {
    wand::SharedDeviceHandle<wand::Buffer> buf;
    std::byte* mapped_ptr{nullptr};
    UINT64 size{0};
    UINT64 current_offset{0};
  };


  [[nodiscard]]
  auto AllocateUploadSpace(UINT64 size, UINT64 alignment) -> UploadAllocation;


  ObserverPtr<wand::GraphicsDevice> device_;
  std::vector<UploadBufferPage> upload_pages_;

  static std::uint64_t const kUploadBufferPageMinSize;
};
}
