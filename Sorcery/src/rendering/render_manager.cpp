#include "render_manager.hpp"

#include "../MemoryAllocation.hpp"
#include "../Util.hpp"


namespace sorcery::rendering {
RenderManager::RenderManager(graphics::GraphicsDevice& device) :
  device_{&device},
  in_flight_frames_fence_{device_->CreateFence(0)} {}


auto RenderManager::BeginNewFrame() -> void {
  ++frame_count_;
  frame_idx_ = (frame_idx_ + 1) % max_frames_in_flight_;
  next_cmd_list_idx_ = 0;

  AgeTempRenderTargets();
  ReleaseOldTempRenderTargets();
}


auto RenderManager::GetCurrentFrameCount() const -> UINT64 {
  return frame_count_;
}


auto RenderManager::GetCurrentFrameIndex() const -> UINT {
  return frame_idx_;
}


auto RenderManager::AcquireCommandList() -> graphics::CommandList& {
  if (next_cmd_list_idx_ >= cmd_lists_.size()) {
    CreateCommandLists(1);
  }
  return *cmd_lists_[next_cmd_list_idx_++][frame_idx_];
}


auto RenderManager::AcquireTemporaryRenderTarget(RenderTarget::Desc const& desc) -> std::shared_ptr<RenderTarget> {
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


auto RenderManager::LoadReadonlyTexture(
  DirectX::ScratchImage const& img) -> graphics::SharedDeviceChildHandle<graphics::Texture> {
  auto& cmd{AcquireCommandList()};

  auto const& meta{img.GetMetadata()};

  graphics::TextureDesc desc;
  desc.width = static_cast<UINT>(meta.width);
  desc.mip_levels = static_cast<UINT16>(meta.mipLevels);
  desc.format = meta.format;
  desc.sample_desc.Count = 1;
  desc.sample_desc.Quality = 0;
  desc.flags = D3D12_RESOURCE_FLAG_NONE;
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

  auto const upload_buf{
    device_->CreateBuffer(graphics::BufferDesc{
      static_cast<UINT>(tex->GetRequiredIntermediateSize()), 0, false, false, false
    }, D3D12_HEAP_TYPE_UPLOAD)
  };

  if (!cmd.Begin(nullptr)) {
    return nullptr;
  }

  std::pmr::vector<D3D12_SUBRESOURCE_DATA> subresource_data{&GetTmpMemRes()};
  subresource_data.reserve(img.GetImageCount());

  for (std::size_t i{0}; i < img.GetImageCount(); i++) {
    auto const subimg{img.GetImages()[i]};
    subresource_data.emplace_back(subimg.pixels, subimg.rowPitch, subimg.slicePitch);
  }

  cmd.UpdateSubresources(*tex, *upload_buf, 0, 0, static_cast<UINT>(img.GetImageCount()), subresource_data.data());

  cmd.Barrier({}, {}, std::array{
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_NO_ACCESS,
      D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE, tex.get(),
      D3D12_BARRIER_SUBRESOURCE_RANGE{
        0, static_cast<UINT>(meta.mipLevels), 0, static_cast<UINT>(std::max(meta.depth, meta.arraySize)), 0, 1
      },
      D3D12_TEXTURE_BARRIER_FLAG_NONE
    }
  });

  if (!cmd.End()) {
    return nullptr;
  }

  auto constexpr init_fence_val{0};
  auto constexpr completed_fence_val{1};
  auto const fence{device_->CreateFence(init_fence_val)};

  device_->ExecuteCommandLists(std::span{&cmd, 1});

  if (!device_->SignalFence(*fence, completed_fence_val)) {
    return nullptr;
  }

  if (!fence->Wait(completed_fence_val)) {
    return nullptr;
  }

  return tex;
}


auto RenderManager::UpdateBuffer(graphics::Buffer const& buf, std::span<std::byte const> const data) -> bool {
  if (buf.GetRequiredIntermediateSize() < data.size()) {
    return false;
  }

  auto const upload_buf{
    device_->CreateBuffer(graphics::BufferDesc{static_cast<UINT>(data.size()), 0, false, false, false},
      D3D12_HEAP_TYPE_UPLOAD)
  };

  if (!upload_buf) {
    return false;
  }

  auto const ptr{upload_buf->Map()};

  if (!ptr) {
    return false;
  }

  std::memcpy(ptr, data.data(), data.size());

  auto& cmd{AcquireCommandList()};

  if (!cmd.Begin(nullptr)) {
    return false;
  }

  cmd.CopyBufferRegion(buf, 0, *upload_buf, 0, data.size());

  if (!cmd.End()) {
    return false;
  }

  device_->ExecuteCommandLists(std::span{&cmd, 1});

  auto constexpr init_fence_val{0};
  auto constexpr final_fence_val{1};

  auto const fence{device_->CreateFence(init_fence_val)};

  if (!fence) {
    return false;
  }

  if (!device_->SignalFence(*fence, final_fence_val)) {
    return false;
  }

  if (!fence->Wait(final_fence_val)) {
    return false;
  }

  return true;
}


auto RenderManager::WaitForInFlightFrames() const -> bool {
  if (!device_->SignalFence(*in_flight_frames_fence_, frame_count_)) {
    return false;
  }
  return in_flight_frames_fence_->Wait(SatSub<UINT64>(frame_count_, max_gpu_queued_frames_));
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


auto RenderManager::ReleaseOldTempRenderTargets() -> void {
  tmp_render_targets_.erase(std::ranges::remove_if(tmp_render_targets_, [](TempRenderTargetRecord const& record) {
    return record.age_in_frames >= max_tmp_rt_age_;
  }).begin(), tmp_render_targets_.end());
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
