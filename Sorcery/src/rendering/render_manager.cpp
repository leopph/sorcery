#include "render_manager.hpp"

#include "../MemoryAllocation.hpp"


namespace sorcery::rendering {
namespace {
std::vector const kQuadPositions{Vector3{-1, 1, 0}, Vector3{-1, -1, 0}, Vector3{1, -1, 0}, Vector3{1, 1, 0}};
std::vector const kQuadUvs{Vector2{0, 0}, Vector2{0, 1}, Vector2{1, 1}, Vector2{1, 0}};
std::vector<std::uint32_t> const kQuadIndices{2, 1, 0, 0, 3, 2};
std::vector const kCubePositions{
  Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f},
  Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, -0.5f}, Vector3{-0.5f, 0.5f, -0.5f},
  Vector3{-0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f},
  Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f},
  Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, -0.5f}, Vector3{-0.5f, -0.5f, -0.5f},
  Vector3{-0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f},
};
std::vector const kCubeUvs{
  Vector2{1, 0}, Vector2{1, 0}, Vector2{0, 0}, Vector2{0, 0}, Vector2{0, 0}, Vector2{1, 0}, Vector2{1, 0},
  Vector2{0, 1}, Vector2{0, 0}, Vector2{0, 0}, Vector2{1, 1}, Vector2{1, 0}, Vector2{1, 1}, Vector2{1, 1},
  Vector2{0, 1}, Vector2{0, 1}, Vector2{0, 1}, Vector2{1, 1}, Vector2{1, 1}, Vector2{0, 0}, Vector2{0, 1},
  Vector2{0, 1}, Vector2{1, 0}, Vector2{1, 1}
};
std::vector<std::uint32_t> const kCubeIndices{
  // Top face
  7, 4, 1, 1, 10, 7,
  // Bottom face
  16, 19, 22, 22, 13, 16,
  // Front face
  23, 20, 8, 8, 11, 23,
  // Back face
  17, 14, 2, 2, 5, 17,
  // Right face
  21, 9, 0, 0, 12, 21,
  // Left face
  15, 3, 6, 6, 18, 15
};
}


RenderManager::RenderManager(graphics::GraphicsDevice& device) {
  device_.Reset(&device);

  default_mtl_.Reset(CreateAndInitialize<Material>());
  default_mtl_->SetGuid(default_material_guid_);
  default_mtl_->SetName("Default Material");
  gResourceManager.Add(default_mtl_.Get());

  std::vector<Vector3> cubeNormals;
  CalculateNormals(kCubePositions, kCubeIndices, cubeNormals);

  std::vector<Vector3> cubeTangents;
  CalculateTangents(kCubePositions, kCubeUvs, kCubeIndices, cubeTangents);

  std::vector<Vector3> quadNormals;
  CalculateNormals(kQuadPositions, kQuadIndices, quadNormals);

  std::vector<Vector3> quadTangents;
  CalculateTangents(kQuadPositions, kQuadUvs, kQuadIndices, quadTangents);

  cube_mesh_.Reset(CreateAndInitialize<Mesh>());
  cube_mesh_->SetGuid(cube_mesh_guid_);
  cube_mesh_->SetName("Cube");
  cube_mesh_->SetPositions(kCubePositions);
  cube_mesh_->SetNormals(std::move(cubeNormals));
  cube_mesh_->SetUVs(kCubeUvs);
  cube_mesh_->SetTangents(std::move(cubeTangents));
  cube_mesh_->SetIndices(kCubeIndices);
  cube_mesh_->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  cube_mesh_->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(kCubeIndices.size()), 0, AABB{}}});
  if (!cube_mesh_->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default cube mesh."};
  }
  gResourceManager.Add(cube_mesh_.Get());

  plane_mesh_.Reset(CreateAndInitialize<Mesh>());
  plane_mesh_->SetGuid(plane_mesh_guid_);
  plane_mesh_->SetName("Plane");
  plane_mesh_->SetPositions(kQuadPositions);
  plane_mesh_->SetNormals(std::move(quadNormals));
  plane_mesh_->SetUVs(kQuadUvs);
  plane_mesh_->SetTangents(std::move(quadTangents));
  plane_mesh_->SetIndices(kQuadIndices);
  plane_mesh_->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  plane_mesh_->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(kQuadIndices.size()), 0, AABB{}}});
  if (!plane_mesh_->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default plane mesh."};
  }
  gResourceManager.Add(plane_mesh_.Get());

  sphere_mesh_.Reset(CreateAndInitialize<Mesh>());
  sphere_mesh_->SetGuid(sphere_mesh_guid_);
  sphere_mesh_->SetName("Sphere");
  std::vector<Vector3> spherePositions;
  std::vector<Vector3> sphereNormals;
  std::vector<Vector3> sphereTangents;
  std::vector<Vector2> sphereUvs;
  std::vector<std::uint32_t> sphereIndices;
  GenerateSphereMesh(1, 50, 50, spherePositions, sphereNormals, sphereUvs, sphereIndices);
  auto const sphereIdxCount{std::size(sphereIndices)};
  CalculateTangents(spherePositions, sphereUvs, sphereIndices, sphereTangents);
  sphere_mesh_->SetPositions(std::move(spherePositions));
  sphere_mesh_->SetNormals(std::move(sphereNormals));
  sphere_mesh_->SetUVs(std::move(sphereUvs));
  sphere_mesh_->SetTangents(std::move(sphereTangents));
  sphere_mesh_->SetIndices(std::move(sphereIndices));
  sphere_mesh_->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  sphere_mesh_->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(sphereIdxCount), 0, AABB{}}});
  if (!sphere_mesh_->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default sphere mesh."};
  }
  gResourceManager.Add(sphere_mesh_.Get());
}


auto RenderManager::BeginNewFrame() -> void {
  frame_idx_ = (frame_idx_ + 1) % max_frames_in_flight_;
  next_cmd_list_idx_ = 0;

  ReleaseTempRenderTargets();
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


auto RenderManager::GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> std::shared_ptr<RenderTarget> {
  for (auto& [rt, lastUseInFrames] : tmp_render_targets_) {
    if (rt->GetDesc() == desc && lastUseInFrames != 0 /*The RT wasn't already handed out this frame*/) {
      lastUseInFrames = 0;
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
    desc.dimension = graphics::TextureDimension::k2D;
    desc.height = static_cast<UINT>(meta.height);
    desc.depth_or_array_size = static_cast<UINT16>(meta.arraySize);
  } else if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D) {
    if (meta.IsCubemap()) {
      desc.dimension = graphics::TextureDimension::kCube;
      desc.height = static_cast<UINT>(meta.height);
      desc.depth_or_array_size = static_cast<UINT16>(meta.arraySize);
    } else {
      desc.dimension = graphics::TextureDimension::k3D;
      desc.height = static_cast<UINT>(meta.height);
      desc.depth_or_array_size = static_cast<UINT16>(meta.depth);
    }
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

  if (!device_->SignalFence(*fence.Get(), completed_fence_val)) {
    return nullptr;
  }

  if (FAILED(fence->SetEventOnCompletion(completed_fence_val, nullptr))) {
    return nullptr;
  }

  return tex;
}


auto RenderManager::UpdateBuffer(graphics::Buffer const& buf, std::span<std::byte const> const data) -> bool {
  if (buf.GetRequiredIntermediateSize() != data.size()) {
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

  cmd.CopyBuffer(buf, *upload_buf);

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

  if (!device_->SignalFence(*fence.Get(), final_fence_val)) {
    return false;
  }

  if (FAILED(fence->SetEventOnCompletion(final_fence_val, nullptr))) {
    return false;
  }

  return true;
}


auto RenderManager::GetDevice() const -> graphics::GraphicsDevice& {
  return *device_;
}


auto RenderManager::GetDefaultMaterial() const noexcept -> ObserverPtr<Material> {
  return default_mtl_;
}


auto RenderManager::GetCubeMesh() const noexcept -> ObserverPtr<Mesh> {
  return cube_mesh_;
}


auto RenderManager::GetPlaneMesh() const noexcept -> ObserverPtr<Mesh> {
  return plane_mesh_;
}


auto RenderManager::GetSphereMesh() const noexcept -> ObserverPtr<Mesh> {
  return sphere_mesh_;
}


auto RenderManager::CreateCommandLists(UINT const count) -> void {
  cmd_lists_.reserve(cmd_lists_.size() + count);

  for (UINT i{0}; i < count; i++) {
    auto& arr{cmd_lists_.emplace_back()};

    for (UINT j{0}; j < max_frames_in_flight_; j++) {
      arr[i] = device_->CreateCommandList();
    }
  }
}


auto RenderManager::ReleaseTempRenderTargets() noexcept -> void {
  std::erase_if(tmp_render_targets_, [](TempRenderTargetRecord& tmpRtRecord) {
    tmpRtRecord.age_in_frames += 1;
    return tmpRtRecord.age_in_frames >= max_tmp_rt_age_;
  });
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
