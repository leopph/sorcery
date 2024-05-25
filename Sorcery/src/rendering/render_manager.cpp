#include "render_manager.hpp"

#include "../MemoryAllocation.hpp"
#include "../Util.hpp"

#include <stdexcept>
#include <utility>


namespace sorcery::rendering {
RenderManager::RenderManager(graphics::GraphicsDevice& device) :
  device_{&device},
  in_flight_frames_fence_{device_->CreateFence(0)},
  upload_fence_{device_->CreateFence(0)} {
  RecreateUploadBuffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
}


auto RenderManager::GetCurrentFrameCount() const -> UINT64 {
  return frame_count_;
}


auto RenderManager::GetCurrentFrameIndex() const -> UINT {
  return frame_idx_;
}


auto RenderManager::AcquireCommandList() -> graphics::CommandList& {
  std::scoped_lock const lck{cmd_list_mutex_};
  if (next_cmd_list_idx_ >= cmd_lists_.size()) {
    CreateCommandLists(1);
  }
  return *cmd_lists_[next_cmd_list_idx_++][frame_idx_];
}


auto RenderManager::AcquireTemporaryRenderTarget(RenderTarget::Desc const& desc) -> std::shared_ptr<RenderTarget> {
  std::scoped_lock const lck{tmp_render_targets_mutex_};
  for (auto& [rt, age_in_frames] : tmp_render_targets_) {
    // An age of 0 means the render target was acquired this frame and is considered in use
    if (age_in_frames > 0 && rt->GetDesc() == desc) {
      age_in_frames = 0;
      return rt;
    }
  }
  return tmp_render_targets_.emplace_back(std::shared_ptr<RenderTarget>{RenderTarget::New(*device_, desc).release()}, 0)
                            .rt;
}


auto RenderManager::UpdateBuffer(graphics::Buffer const& buf, UINT const byte_offset,
                                 std::span<std::byte const> const data) -> void {
  if (buf.GetDesc().size - byte_offset < data.size()) {
    throw std::runtime_error{"Failed to update buffer: the provided data does not fit in the destination buffer."};
  }

  std::scoped_lock const lck{upload_mutex_};

  if (auto const upload_buf_size{upload_buf_->GetDesc().size};
    upload_buf_size - upload_buf_current_offset_ < data.size()) {
    WaitForAllUploads();

    if (upload_buf_size < data.size()) {
      RecreateUploadBuffer(data.size());
    }
  }

  std::memcpy(upload_ptr_ + upload_buf_current_offset_, data.data(), data.size());

  auto& cmd{AcquireCommandList()};
  cmd.Begin(nullptr);
  cmd.CopyBufferRegion(buf, byte_offset, *upload_buf_, upload_buf_current_offset_, data.size());
  cmd.End();

  device_->ExecuteCommandLists(std::span{&cmd, 1});
  device_->SignalFence(*upload_fence_);

  upload_buf_current_offset_ += data.size();
}


auto RenderManager::UpdateTexture(graphics::Texture const& tex, UINT const subresource_offset,
                                  std::span<D3D12_SUBRESOURCE_DATA const> const data) -> void {
  UINT64 tex_size;
  device_->GetCopyableFootprints(tex.GetDesc(), subresource_offset, static_cast<UINT>(data.size()), 0, nullptr, nullptr,
    nullptr, &tex_size);

  std::scoped_lock const lck{upload_mutex_};

  if (auto const upload_buf_size{upload_buf_->GetDesc().size};
    upload_buf_size - upload_buf_current_offset_ < tex_size) {
    WaitForAllUploads();

    if (upload_buf_size < tex_size) {
      RecreateUploadBuffer(tex_size);
    }
  }

  std::pmr::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts{&GetSingleFrameLinearMemory()};
  layouts.resize(data.size());

  std::pmr::vector<UINT> row_counts{&GetSingleFrameLinearMemory()};
  row_counts.resize(data.size());

  std::pmr::vector<UINT64> row_sizes{&GetSingleFrameLinearMemory()};
  row_sizes.resize(data.size());

  device_->GetCopyableFootprints(tex.GetDesc(), subresource_offset, static_cast<UINT>(data.size()),
    upload_buf_current_offset_, layouts.data(), row_counts.data(), row_sizes.data(), nullptr);

  for (std::size_t i{0}; i < data.size(); i++) {
    D3D12_MEMCPY_DEST const dst{
      upload_ptr_ + layouts[i].Offset, layouts[i].Footprint.RowPitch,
      static_cast<std::size_t>(layouts[i].Footprint.RowPitch) * static_cast<std::size_t>(row_counts[i])
    };
    MemcpySubresource(&dst, &data[i], row_sizes[i], row_counts[i], layouts[i].Footprint.Depth);
  }

  auto& cmd{AcquireCommandList()};
  cmd.Begin(nullptr);

  for (UINT i{0}; i < static_cast<UINT>(data.size()); i++) {
    cmd.CopyTextureRegion(tex, subresource_offset + i, 0, 0, 0, *upload_buf_, layouts[i]);
  }

  cmd.End();

  device_->ExecuteCommandLists(std::span{&cmd, 1});
  device_->SignalFence(*upload_fence_);

  upload_buf_current_offset_ += tex_size;
}


auto RenderManager::CreateReadOnlyTexture(
  DirectX::ScratchImage const& img) -> graphics::SharedDeviceChildHandle<graphics::Texture> {
  auto const& meta{img.GetMetadata()};

  graphics::TextureDesc desc;
  desc.width = static_cast<UINT>(meta.width);
  desc.mip_levels = static_cast<UINT16>(meta.mipLevels);
  desc.format = meta.format;
  desc.sample_count = 1;
  desc.depth_stencil = false;
  desc.render_target = false;
  desc.shader_resource = true;
  desc.unordered_access = false;

  if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE1D) {
    desc.dimension = graphics::TextureDimension::k1D;
    desc.height = 1;
    desc.depth_or_array_size = static_cast<UINT16>(meta.arraySize);
  } else if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D) {
    if (meta.IsCubemap()) {
      desc.dimension = graphics::TextureDimension::kCube;
    } else {
      desc.dimension = graphics::TextureDimension::k2D;
    }
    desc.height = static_cast<UINT>(meta.height);
    desc.depth_or_array_size = static_cast<UINT16>(meta.arraySize);
  } else if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D) {
    desc.dimension = graphics::TextureDimension::k3D;
    desc.height = static_cast<UINT>(meta.height);
    desc.depth_or_array_size = static_cast<UINT16>(meta.depth);
  }

  auto tex{device_->CreateTexture(desc, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr)};

  std::pmr::vector<D3D12_SUBRESOURCE_DATA> subresource_data{&GetSingleFrameLinearMemory()};
  subresource_data.reserve(img.GetImageCount());

  for (std::size_t i{0}; i < img.GetImageCount(); i++) {
    auto const subimg{img.GetImages()[i]};
    subresource_data.emplace_back(subimg.pixels, static_cast<LONG_PTR>(subimg.rowPitch),
      static_cast<LONG_PTR>(subimg.slicePitch));
  }

  UpdateTexture(*tex, 0, subresource_data);

  auto& cmd{AcquireCommandList()};
  cmd.Begin(nullptr);
  cmd.Barrier({}, {}, std::array{
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_COPY_DEST,
      D3D12_BARRIER_ACCESS_SHADER_RESOURCE, D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE,
      tex.get(), {0xffffffff}
    }
  });
  cmd.End();
  device_->ExecuteCommandLists(std::span{&cmd, 1});

  return tex;
}


auto RenderManager::KeepAliveWhileInUse(graphics::SharedDeviceChildHandle<graphics::Buffer> buf) -> void {
  std::scoped_lock const lock{keep_alive_buffers_mutex_};
  buffers_to_keep_alive_.emplace_back(std::move(buf), 0);
}


auto RenderManager::EndFrame() -> void {
  WaitForInFlightFrames();
  UpdateCounters();
  AgeTempRenderTargets();
  AgeKeepAliveBuffers();
  ReleaseOldTempRenderTargets();
  ReleaseUnusedBuffers();
}


auto RenderManager::CreateCommandLists(UINT const count) -> void {
  cmd_lists_.reserve(cmd_lists_.size() + count);

  for (UINT i{0}; i < count; i++) {
    auto& arr{cmd_lists_.emplace_back()};

    for (UINT j{0}; j < max_frames_in_flight_; j++) {
      arr[j] = device_->CreateCommandList();
    }
  }
}


auto RenderManager::AgeTempRenderTargets() -> void {
  std::ranges::for_each(tmp_render_targets_, [](TempRenderTargetRecord& record) {
    ++record.age_in_frames;
  });
}


auto RenderManager::AgeKeepAliveBuffers() -> void {
  std::scoped_lock const lock{keep_alive_buffers_mutex_};
  std::ranges::for_each(buffers_to_keep_alive_, [](KeepAliveBufferRecord& record) {
    ++record.age;
  });
}


auto RenderManager::ReleaseOldTempRenderTargets() -> void {
  tmp_render_targets_.erase(std::ranges::remove_if(tmp_render_targets_, [](TempRenderTargetRecord const& record) {
    return record.age_in_frames >= max_tmp_rt_age_;
  }).begin(), tmp_render_targets_.end());
}


auto RenderManager::ReleaseUnusedBuffers() -> void {
  std::scoped_lock const lock{keep_alive_buffers_mutex_};
  buffers_to_keep_alive_.erase(std::ranges::remove_if(buffers_to_keep_alive_, [](KeepAliveBufferRecord const& record) {
    // Work could have still been dispatched during the frame the buffers was passed to keep alive.
    // This is why we wait for all gpu queued frames to complete as well as the one we received the buffer in.
    return record.age > max_frames_in_flight_;
  }).begin(), buffers_to_keep_alive_.end());
}


auto RenderManager::RecreateUploadBuffer(UINT64 const size) -> void {
  upload_buf_ = device_->CreateBuffer(graphics::BufferDesc{size, 0, false, false, false}, D3D12_HEAP_TYPE_UPLOAD);
  upload_buf_->SetDebugName(L"Render Manager Upload Buffer");
  upload_ptr_ = static_cast<std::byte*>(upload_buf_->Map());
}


auto RenderManager::WaitForAllUploads() -> void {
  upload_buf_current_offset_ = 0;
  upload_fence_->Wait(upload_fence_->GetNextValue() - 1);
}


auto RenderManager::WaitForInFlightFrames() const -> void {
  auto const fence_val{in_flight_frames_fence_->GetNextValue()};
  device_->SignalFence(*in_flight_frames_fence_);
  in_flight_frames_fence_->Wait(SatSub<UINT64>(fence_val, max_gpu_queued_frames_));
}


auto RenderManager::UpdateCounters() -> void {
  ++frame_count_;
  frame_idx_ = (frame_idx_ + 1) % max_frames_in_flight_;
  next_cmd_list_idx_ = 0;
}


auto TransformProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4 {
  return proj_mtx * Matrix4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 1, 1};
}


auto GenerateSphereMesh(float const radius, int const latitudes, int const longitudes, std::vector<Vector3>& vertices,
                        std::vector<Vector3>& normals, std::vector<Vector2>& uvs,
                        std::vector<std::uint32_t>& indices) -> void {
  // Based on: https://gist.github.com/Pikachuxxxx/5c4c490a7d7679824e0e18af42918efc

  auto const deltaLatitude{PI / static_cast<float>(latitudes)};
  auto const deltaLongitude{2 * PI / static_cast<float>(longitudes)};

  for (int i = 0; i <= latitudes; i++) {
    auto const latitudeAngle{PI / 2 - static_cast<float>(i) * deltaLatitude};

    float const xz{radius * std::cos(latitudeAngle)};
    float const y{radius * std::sin(latitudeAngle)};

    /* We add (latitudes + 1) vertices per longitude because of equator,
     * the North pole and South pole are not counted here, as they overlap.
     * The first and last vertices have same position and normal, but
     * different tex coords. */
    for (int j = 0; j <= longitudes; j++) {
      auto const longitudeAngle{static_cast<float>(j) * deltaLongitude};

      auto const x{xz * std::cos(longitudeAngle)};
      auto const z{xz * std::sin(longitudeAngle)};
      auto const u{static_cast<float>(j) / static_cast<float>(longitudes)};
      auto const v{static_cast<float>(i) / static_cast<float>(latitudes)};
      vertices.emplace_back(x, y, z);
      uvs.emplace_back(u, v);
      normals.emplace_back(x / radius, y / radius, z / radius);
    }
  }


  /*  Indices
   *  k1--k1+1
   *  |  / |
   *  | /  |
   *  k2--k2+1 */
  for (int i = 0; i < latitudes; ++i) {
    unsigned int v1 = i * (longitudes + 1);
    unsigned int v2 = v1 + longitudes + 1;

    // 2 Triangles per latitude block excluding the first and last longitudes blocks
    for (int j = 0; j < longitudes; j++, v1++, v2++) {
      if (i != 0) {
        indices.push_back(v1);
        indices.push_back(v1 + 1);
        indices.push_back(v2);
      }

      if (i != latitudes - 1) {
        indices.push_back(v1 + 1);
        indices.push_back(v2 + 1);
        indices.push_back(v2);
      }
    }
  }
}
}
