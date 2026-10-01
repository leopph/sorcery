#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include "../observer_ptr.hpp"
#include "wand/wand.hpp"


namespace sorcery::rendering {
class FrameUploader {
public:
  explicit FrameUploader(wand::GraphicsDevice& device);

  auto UploadBuffer(wand::SharedDeviceChildHandle<wand::Buffer> const& buf, UINT64 byte_offset,
                    std::span<std::byte const> data) -> void;

  [[nodiscard]]
  auto IsEmpty() const -> bool;
  auto Record(wand::CommandList& cmd) const -> void;
  auto Reset() -> void;

private:
  struct BufferUpload {
    wand::SharedDeviceChildHandle<wand::Buffer> buf;
    UINT64 data_size{};
    UINT64 page_offset{};
    UINT64 dst_offset{};
    std::size_t page_idx{};
  };


  struct UploadBufferPage {
    wand::SharedDeviceChildHandle<wand::Buffer> buf;
    std::byte* mapped_ptr{nullptr};
    UINT64 size{0};
    UINT64 current_offset{0};
  };


  ObserverPtr<wand::GraphicsDevice> device_;
  std::vector<BufferUpload> buffer_uploads_;
  std::vector<UploadBufferPage> upload_pages_;

  static std::uint64_t const kUploadBufferPageMinSize;
};
}
