#pragma once

#include <cstddef>
#include <span>
#include <variant>
#include <vector>

#include "../observer_ptr.hpp"
#include "wand/wand.hpp"


namespace sorcery::rendering {
class FrameUploader {
public:
  explicit FrameUploader(wand::GraphicsDevice& device);

  auto UploadBuffer(wand::SharedDeviceHandle<wand::Buffer> const& buf, UINT64 byte_offset,
                    std::span<std::byte const> data) -> void;

  auto UploadTexture(wand::SharedDeviceHandle<wand::Texture> const& tex, UINT first_subresource,
                     std::span<D3D12_SUBRESOURCE_DATA const> data) -> void;

  [[nodiscard]]
  auto IsEmpty() const -> bool;
  auto Record(wand::CommandList& cmd) const -> void;
  auto Reset() -> void;

private:
  struct BufferUpload {
    wand::SharedDeviceHandle<wand::Buffer> buf;
    UINT64 data_size{};
    UINT64 page_offset{};
    UINT64 dst_offset{};
    std::size_t page_idx{};
  };


  struct TextureUpload {
    wand::SharedDeviceHandle<wand::Texture> tex;
    UINT first_subresource{};
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
    std::size_t page_idx{};
  };


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
  std::vector<std::variant<BufferUpload, TextureUpload>> uploads_;
  std::vector<UploadBufferPage> upload_pages_;

  static std::uint64_t const kUploadBufferPageMinSize;
};
}
