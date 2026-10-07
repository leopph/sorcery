#include "frame_uploader.hpp"

#include <algorithm>
#include <cstring>
#include <optional>
#include <stdexcept>


namespace sorcery::rendering {
FrameUploader::FrameUploader(wand::GraphicsDevice& device) :
  device_{&device} {}


auto FrameUploader::UploadBuffer(wand::SharedDeviceHandle<wand::Buffer> const& buf, UINT64 const byte_offset,
                                 std::span<std::byte const> const data) -> void {
  if (!buf) {
    throw std::runtime_error{"Failed to update buffer: the provided buffer is null."};
  }

  if (auto const buf_size = buf->GetDesc().size;
    byte_offset > buf_size || data.size() > buf_size - byte_offset) {
    throw std::runtime_error{"Failed to update buffer: the provided data does not fit in the destination buffer."};
  }

  // Look for an existing buffer page that has enough space to hold the data.

  std::optional<std::size_t> found_page_idx;

  for (auto i = 0uz; i < upload_pages_.size(); ++i) {
    if (upload_pages_[i].size - upload_pages_[i].current_offset >= data.size()) {
      found_page_idx = i;
      break;
    }
  }

  // If no existing page was found, create a new one.

  if (!found_page_idx) {
    auto& new_page{upload_pages_.emplace_back()};

    auto const buf_size = std::max(data.size(), kUploadBufferPageMinSize);

    new_page.buf = device_->CreateBuffer(wand::BufferDesc{
      .size = buf_size,
      .usage = wand::BufferUsage::kCopySource
    }, wand::CpuAccess::kWrite);

    new_page.buf->SetDebugName(L"Frame Uploader Upload Buffer");
    new_page.mapped_ptr = static_cast<std::byte*>(new_page.buf->Map());
    new_page.size = buf_size;
    new_page.current_offset = 0;

    found_page_idx = upload_pages_.size() - 1;
  }

  auto& page = upload_pages_[*found_page_idx];

  buffer_uploads_.emplace_back(buf, data.size(), page.current_offset, byte_offset,
    *found_page_idx);

  std::memcpy(page.mapped_ptr + page.current_offset, data.data(), data.size());
  page.current_offset += data.size();
}


auto FrameUploader::IsEmpty() const -> bool {
  return buffer_uploads_.empty();
}


auto FrameUploader::Record(wand::CommandList& cmd) const -> void {
  for (auto const& upload : buffer_uploads_) {
    cmd.CopyBufferRegion(*upload.buf, upload.dst_offset, *upload_pages_[upload.page_idx].buf, upload.page_offset,
      upload.data_size);
  }
}


auto FrameUploader::Reset() -> void {
  buffer_uploads_.clear();

  for (auto& page : upload_pages_) {
    page.current_offset = 0;
  }
}


std::uint64_t const FrameUploader::kUploadBufferPageMinSize{4ull * 1024ull * 1024ull}; // 4 MB
}
