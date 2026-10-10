#include "frame_uploader.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>


namespace sorcery::rendering {
FrameUploader::FrameUploader(wand::GraphicsDevice& device) :
  device_{&device} {}


auto FrameUploader::UploadBuffer(wand::SharedDeviceHandle<wand::Buffer> const& buf, UINT64 const byte_offset,
                                 std::span<std::byte const> const data) -> void {
  if (!buf) {
    throw std::runtime_error{"Failed to upload buffer: the provided buffer is null."};
  }

  if (data.empty()) {
    return;
  }

  if (auto const buf_size = buf->GetDesc().size;
    byte_offset > buf_size || data.size() > buf_size - byte_offset) {
    throw std::runtime_error{"Failed to upload buffer: the provided data does not fit in the destination buffer."};
  }

  auto const alloc = AllocateUploadSpace(data.size(), 1);
  auto const& page = upload_pages_[alloc.page_idx];

  std::memcpy(page.mapped_ptr + alloc.offset, data.data(), data.size());

  uploads_.emplace_back(BufferUpload{
    .buf = buf,
    .data_size = data.size(),
    .page_offset = alloc.offset,
    .dst_offset = byte_offset,
    .page_idx = alloc.page_idx
  });
}


auto FrameUploader::UploadTexture(wand::SharedDeviceHandle<wand::Texture> const& tex, UINT const first_subresource,
                                  std::span<D3D12_SUBRESOURCE_DATA const> const data) -> void {
  if (!tex) {
    throw std::runtime_error{"Failed to upload texture: the provided texture is null."};
  }

  if (data.empty()) {
    return;
  }

  if (data.size() > std::numeric_limits<UINT>::max()) {
    throw std::runtime_error{"Failed to upload texture: too many subresources."};
  }

  auto const& tex_desc = tex->GetDesc();

  if (tex_desc.sample_count != 1) {
    throw std::runtime_error{"Failed to upload texture: texture is multisampled."};
  }

  auto const subres_count = static_cast<UINT>(data.size());

  std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(subres_count);
  std::vector<UINT> row_counts(subres_count);
  std::vector<UINT64> row_sizes(subres_count);

  UINT64 required_size{0};

  device_->GetCopyableFootprints(tex_desc, first_subresource, subres_count, 0, layouts.data(), row_counts.data(),
    row_sizes.data(), &required_size);

  auto const alloc = AllocateUploadSpace(required_size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
  auto const& page = upload_pages_[alloc.page_idx];

  for (auto i = 0uz; i < data.size(); ++i) {
    auto& layout = layouts[i];

    layout.Offset += alloc.offset;

    D3D12_MEMCPY_DEST const dst{
      .pData = page.mapped_ptr + layout.Offset,
      .RowPitch = layout.Footprint.RowPitch,
      .SlicePitch = static_cast<SIZE_T>(layout.Footprint.RowPitch) * row_counts[i]
    };

    MemcpySubresource(&dst, &data[i], row_sizes[i], row_counts[i], layout.Footprint.Depth);
  }

  uploads_.emplace_back(TextureUpload{
    .tex = tex,
    .first_subresource = first_subresource,
    .layouts = std::move(layouts),
    .page_idx = alloc.page_idx
  });
}


auto FrameUploader::IsEmpty() const -> bool {
  return uploads_.empty();
}


auto FrameUploader::Record(wand::CommandList& cmd) const -> void {
  for (auto const& upload : uploads_) {
    if (auto const* const buf_upload = std::get_if<BufferUpload>(&upload)) {
      cmd.CopyBufferRegion(*buf_upload->buf, buf_upload->dst_offset, *upload_pages_[buf_upload->page_idx].buf,
        buf_upload->page_offset, buf_upload->data_size);
    } else {
      auto const& tex_upload = std::get<TextureUpload>(upload);
      auto const& src = *upload_pages_[tex_upload.page_idx].buf;

      for (auto i = 0uz; i < tex_upload.layouts.size(); ++i) {
        cmd.CopyTextureRegion(*tex_upload.tex, tex_upload.first_subresource + static_cast<UINT>(i), 0, 0, 0, src,
          tex_upload.layouts[i]);
      }
    }
  }
}


auto FrameUploader::Reset() -> void {
  uploads_.clear();

  for (auto& page : upload_pages_) {
    page.current_offset = 0;
  }
}


auto FrameUploader::AllocateUploadSpace(UINT64 const size, UINT64 const alignment) -> UploadAllocation {
  assert(alignment > 0);

  for (auto i = 0uz; i < upload_pages_.size(); ++i) {
    auto& page = upload_pages_[i];

    auto const padding = (alignment - page.current_offset % alignment) % alignment;
    auto const available_space = page.size - page.current_offset;

    if (padding <= available_space && size <= available_space - padding) {
      auto const alloc_offset = page.current_offset + padding;
      page.current_offset = alloc_offset + size;

      return UploadAllocation{
        .page_idx = i,
        .offset = alloc_offset
      };
    }
  }

  UploadBufferPage page;

  page.size = std::max(size, kUploadBufferPageMinSize);

  page.buf = device_->CreateBuffer(wand::BufferDesc{
    .size = page.size,
    .usage = wand::BufferUsage::kCopySource
  }, wand::CpuAccess::kWrite);

  page.buf->SetDebugName(L"Frame Uploader Upload Buffer");
  page.mapped_ptr = static_cast<std::byte*>(page.buf->Map());
  page.current_offset = size;

  upload_pages_.emplace_back(std::move(page));

  return UploadAllocation{
    .page_idx = upload_pages_.size() - 1,
    .offset = 0
  };
}


std::uint64_t const FrameUploader::kUploadBufferPageMinSize{4ull * 1024ull * 1024ull}; // 4 MB
}
