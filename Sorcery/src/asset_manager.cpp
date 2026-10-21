#include "sorcery/host/assets/asset_manager.hpp"

#include <cassert>
#include <ranges>
#include <utility>

#include <DirectXTex.h>
#include <wrl/client.h>

#include "mesh_generation.hpp"
#include "sorcery/mesh_utils.hpp"
#include "sorcery/reflection.hpp"
#include "sorcery/util.hpp"
#include "sorcery/assets/prefab.hpp"
#include "sorcery/assets/Scene.hpp"
#include "sorcery/host/job_system.hpp"
#include "sorcery/host/assets/asset_package.hpp"

using Microsoft::WRL::ComPtr;


namespace sorcery {
namespace {
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
}


AssetManager::AssetManager(JobSystem& job_system) :
  job_system_{&job_system} {}


auto AssetManager::Unload(AssetId const& asset_id) -> void {
  auto assets{loaded_assets_.Lock()};

  if (auto const it{assets->find(asset_id)}; it != std::end(*assets)) {
    assets->erase(it);
  }
}


auto AssetManager::UnloadAll() -> void {
  loaded_assets_.Lock()->clear();
}


auto AssetManager::IsLoaded(AssetId const& asset_id) -> bool {
  for (auto const& asset : default_assets_) {
    if (asset->GetAssetId() == asset_id) {
      return true;
    }
  }

  return loaded_assets_.LockShared()->contains(asset_id);
}


auto AssetManager::UpdateMappings(std::map<AssetId, AssetDescription> asset_mappings,
                                  std::map<Guid, std::filesystem::path> file_mappings) -> void {
  while (true) {
    auto self_asset_mappings{asset_mappings_.TryLock()};
    auto self_file_mappings{file_mappings_.TryLock()};

    if (self_asset_mappings && self_file_mappings) {
      **self_asset_mappings = std::move(asset_mappings);
      **self_file_mappings = std::move(file_mappings);
      break;
    }
  }
}


auto AssetManager::GetInfoForAssetsOfType(rttr::type const& type, std::vector<AssetInfo>& out) -> void {
  // Default assets
  for (auto const& asset : default_assets_) {
    if (auto const asset_type{rttr::type::get(*asset)}; asset_type.is_derived_from(type)) {
      out.emplace_back(asset->GetAssetId(), std::string{asset->GetName()}, asset_type);
    }
  }

  // File mappings
  for (auto const& [guid, desc] : *asset_mappings_.LockShared()) {
    if (desc.type.is_derived_from(type)) {
      out.emplace_back(guid, desc.name, desc.type);
    }
  }

  // Other, loaded assets that don't come from files
  for (auto const& asset : *loaded_assets_.LockShared()) {
    auto contains{false};
    for (auto const& asset_info : out) {
      if (asset_info.id == asset->GetAssetId()) {
        contains = true;
        break;
      }
    }

    if (!contains && rttr::type::get(*asset).is_derived_from(type)) {
      out.emplace_back(asset->GetAssetId(), std::string{asset->GetName()}, asset->get_type());
    }
  }
}


auto AssetManager::GetDefaultMaterial() const noexcept -> AssetRef<Material> {
  return MakeAssetRef(MakeObserver(default_mtl_.get()));
}


auto AssetManager::GetCubeMesh() const noexcept -> AssetRef<Mesh> {
  return MakeAssetRef(MakeObserver(cube_mesh_.get()));
}


auto AssetManager::GetPlaneMesh() const noexcept -> AssetRef<Mesh> {
  return MakeAssetRef(MakeObserver(plane_mesh_.get()));
}


auto AssetManager::GetSphereMesh() const noexcept -> AssetRef<Mesh> {
  return MakeAssetRef(MakeObserver(sphere_mesh_.get()));
}


auto AssetManager::CreateDefaultAssets() -> void {
  if (!default_mtl_) {
    default_mtl_ = std::make_unique<Material>();
    default_mtl_->SetAssetId(default_mtl_asset_id_);
    default_mtl_->SetName("Default Material");
    default_assets_.emplace_back(default_mtl_.get());
  }

  if (!cube_mesh_) {
    MeshData cube_data;

    cube_data.positions = kCubePositions;
    CalculateNormals(kCubePositions, kCubeIndices, cube_data.normals);
    CalculateTangents(kCubePositions, kCubeUvs, kCubeIndices, cube_data.tangents);
    cube_data.uvs = kCubeUvs;

    if (!ComputeMeshlets<std::uint32_t, Vector3>(kCubeIndices, kCubePositions, cube_data.meshlets,
      cube_data.vertex_indices, cube_data.triangle_indices, cube_data.cull_data)) {
      throw std::runtime_error{"Failed to compute meshlets for default cube mesh."};
    }

    cube_data.material_slots.emplace_back("Material");
    cube_data.submeshes.emplace_back(0, static_cast<std::uint32_t>(cube_data.meshlets.size()), 0, 0,
      AABB::FromVertices(kCubePositions));
    cube_data.bounds = cube_data.submeshes[0].bounds;
    cube_data.idx32 = true;

    cube_mesh_ = std::make_unique<Mesh>(cube_data, CpuResidencyPolicy::kReleaseAfterUpload);
    cube_mesh_->SetAssetId(cube_mesh_asset_id_);
    cube_mesh_->SetName("Cube");
    default_assets_.emplace_back(cube_mesh_.get());
  }

  if (!plane_mesh_) {
    MeshData plane_data;

    plane_data.positions = kQuadPositions;
    CalculateNormals(kQuadPositions, kQuadIndices, plane_data.normals);
    CalculateTangents(kQuadPositions, kQuadUvs, kQuadIndices, plane_data.tangents);
    plane_data.uvs = kQuadUvs;

    if (!ComputeMeshlets<std::uint32_t, Vector3>(kQuadIndices, kQuadPositions, plane_data.meshlets,
      plane_data.vertex_indices, plane_data.triangle_indices, plane_data.cull_data)) {
      throw std::runtime_error{"Failed to compute meshlets for default plane mesh."};
    }

    plane_data.material_slots.emplace_back("Material");
    plane_data.submeshes.emplace_back(0, static_cast<std::uint32_t>(plane_data.meshlets.size()), 0, 0,
      AABB::FromVertices(kQuadPositions));
    plane_data.bounds = plane_data.submeshes[0].bounds;
    plane_data.idx32 = true;

    plane_mesh_ = std::make_unique<Mesh>(plane_data, CpuResidencyPolicy::kReleaseAfterUpload);
    plane_mesh_->SetAssetId(plane_mesh_asset_id_);
    plane_mesh_->SetName("Plane");
    default_assets_.emplace_back(plane_mesh_.get());
  }

  if (!sphere_mesh_) {
    MeshData sphere_data;
    std::vector<std::uint32_t> sphere_indices;

    GenerateSphereMesh(1, 50, 50, sphere_data.positions, sphere_data.normals, sphere_data.uvs, sphere_indices);
    CalculateTangents(sphere_data.positions, sphere_data.uvs, sphere_indices, sphere_data.tangents);

    if (!ComputeMeshlets<std::uint32_t, Vector3>(sphere_indices, sphere_data.positions, sphere_data.meshlets,
      sphere_data.vertex_indices, sphere_data.triangle_indices, sphere_data.cull_data)) {
      throw std::runtime_error{"Failed to compute meshlets for default sphere mesh."};
    }

    sphere_data.material_slots.emplace_back("Material");
    sphere_data.submeshes.emplace_back(0, static_cast<std::uint32_t>(sphere_data.meshlets.size()), 0, 0,
      AABB::FromVertices(sphere_data.positions));
    sphere_data.bounds = sphere_data.submeshes[0].bounds;
    sphere_data.idx32 = true;

    sphere_mesh_ = std::make_unique<Mesh>(sphere_data, CpuResidencyPolicy::kReleaseAfterUpload);
    sphere_mesh_->SetAssetId(sphere_mesh_asset_id_);
    sphere_mesh_->SetName("Sphere");
    default_assets_.emplace_back(sphere_mesh_.get());
  }
}


auto AssetManager::AssetIdLess::operator()(std::unique_ptr<Asset> const& lhs,
                                           std::unique_ptr<Asset> const& rhs) const noexcept -> bool {
  return lhs->GetAssetId() < rhs->GetAssetId();
}


auto AssetManager::AssetIdLess::operator()(std::unique_ptr<Asset> const& lhs,
                                           AssetId const& rhs) const noexcept -> bool {
  return lhs->GetAssetId() < rhs;
}


auto AssetManager::AssetIdLess::operator()(AssetId const& lhs,
                                           std::unique_ptr<Asset> const& rhs) const noexcept -> bool {
  return lhs < rhs->GetAssetId();
}


auto AssetManager::InternalLoadAsset(AssetId const& asset_id,
                                     AssetDescription const& desc) -> ObjectPtr<Asset> {
  ObserverPtr<Job> loader_job;
  auto created_job{false};

  ScopedDefer const erase_created_job{
    [&]() noexcept {
      if (created_job) { loader_jobs_.Lock()->erase(asset_id); }
    }
  };

  struct JobData {
    AssetId const* asset_id;
    AssetDescription const* desc;
    std::filesystem::path path_abs;
  };

  JobData job_data;

  {
    auto loader_jobs{loader_jobs_.Lock()};

    if (auto const job_it{loader_jobs->find(asset_id)}; job_it != loader_jobs->end()) {
      loader_job = job_it->second;
    } else {
      auto const file_mappings{file_mappings_.LockShared()};

      if (auto const file_it{file_mappings->find(asset_id.GetGuid())}; file_it != file_mappings->end()) {
        job_data.asset_id = &asset_id;
        job_data.desc = &desc;
        job_data.path_abs = file_it->second;

        loader_job = job_system_->CreateJob([this, &job_data] {
          std::unique_ptr<Asset> asset;

          YamlDeserializeContext const ctx{
            .current_guid = job_data.asset_id->GetGuid()
          };

          if (job_data.path_abs.extension() == EXTERNAL_ASSET_EXT) {
            auto const subasset{
              LoadBinaryAssetPackageSubasset(job_data.path_abs, job_data.asset_id->GetIdxInFile())
            };

            if (!subasset) {
              return;
            }

            switch (subasset->payload_kind) {
              case AssetPackagePayloadKind::kTexture: {
                asset = LoadTexture(subasset->bytes);
                break;
              }

              case AssetPackagePayloadKind::kMesh: {
                asset = LoadMesh(subasset->bytes);
                break;
              }

              case AssetPackagePayloadKind::kMaterial: {
                asset = LoadMaterial(subasset->bytes, ctx);
                break;
              }

              case AssetPackagePayloadKind::kPrefab: {
                asset = LoadPrefab(subasset->bytes, ctx);
              }

              case AssetPackagePayloadKind::kInvalid: {
                // TODO Log error
                break;
              }
            }
          } else if (job_data.path_abs.extension() == SCENE_ASSET_EXT) {
            auto scene = std::make_unique<Scene>();
            scene->Deserialize(YAML::LoadFile(job_data.path_abs.string()), ctx);
            asset = std::move(scene);
          } else if (job_data.path_abs.extension() == MATERIAL_ASSET_EXT) {
            auto mtl = std::make_unique<Material>();
            mtl->Deserialize(YAML::LoadFile(job_data.path_abs.string()), ctx);
            asset = std::move(mtl);
          }

          if (asset) {
            asset->SetAssetId(*job_data.asset_id);
            asset->SetName(job_data.desc->name);

            auto const [it, inserted]{loaded_assets_.Lock()->emplace(std::move(asset))};
            assert(inserted);
          }
        });
        created_job = true;
        loader_jobs->emplace(asset_id, loader_job);
        job_system_->Submit(loader_job);
      }
    }
  }

  assert(loader_job);
  job_system_->Wait(loader_job);

  {
    auto const assets{loaded_assets_.LockShared()};
    auto const it{assets->find(asset_id)};
    return MakeObjectPtr(MakeObserver(it != assets->end() ? it->get() : nullptr));
  }
}


auto AssetManager::LoadTexture(std::span<std::byte const> const bytes) -> MaybeNull<std::unique_ptr<Asset>> {
  DirectX::TexMetadata meta;
  DirectX::ScratchImage img;
  if (FAILED(LoadFromDDSMemory(bytes.data(), bytes.size(), DirectX::DDS_FLAGS_NONE, &meta, img))) {
    return nullptr;
  }

  std::unique_ptr<Asset> ret;

  if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D) {
    if (meta.IsCubemap()) {
      ret = std::make_unique<Cubemap>(std::move(img), CpuResidencyPolicy::kReleaseAfterUpload);
    } else {
      ret = std::make_unique<Texture2D>(std::move(img), CpuResidencyPolicy::kReleaseAfterUpload);
    }
  } else {
    return nullptr;
  }

  return ret;
}


auto AssetManager::LoadMesh(std::span<std::byte const> const bytes) -> MaybeNull<std::unique_ptr<Asset>> {
  auto cur_bytes{as_bytes(std::span{bytes})};

  // Element counts

  std::uint64_t vert_count;

  if (!DeserializeFromBinary(cur_bytes, vert_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof vert_count);
  std::uint64_t meshlet_count;

  if (!DeserializeFromBinary(cur_bytes, meshlet_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof meshlet_count);
  std::uint64_t vtx_idx_count;

  if (!DeserializeFromBinary(cur_bytes, vtx_idx_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof vtx_idx_count);
  std::uint64_t prim_idx_count;

  if (!DeserializeFromBinary(cur_bytes, prim_idx_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof prim_idx_count);
  std::uint64_t material_slot_count;

  if (!DeserializeFromBinary(cur_bytes, material_slot_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof material_slot_count);
  std::uint64_t submesh_count;

  if (!DeserializeFromBinary(cur_bytes, submesh_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof submesh_count);
  std::uint64_t anim_count;

  if (!DeserializeFromBinary(cur_bytes, anim_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof anim_count);
  std::uint64_t skeleton_size;

  if (!DeserializeFromBinary(cur_bytes, skeleton_size)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof skeleton_size);
  std::uint64_t bone_count;

  if (!DeserializeFromBinary(cur_bytes, bone_count)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof bone_count);
  MeshData mesh_data;

  // Geometry data

  mesh_data.positions.resize(vert_count);
  std::memcpy(mesh_data.positions.data(), cur_bytes.data(), vert_count * sizeof(Vector3));
  cur_bytes = cur_bytes.subspan(vert_count * sizeof(Vector3));

  mesh_data.normals.resize(vert_count);
  std::memcpy(mesh_data.normals.data(), cur_bytes.data(), vert_count * sizeof(Vector3));
  cur_bytes = cur_bytes.subspan(vert_count * sizeof(Vector3));

  mesh_data.tangents.resize(vert_count);
  std::memcpy(mesh_data.tangents.data(), cur_bytes.data(), vert_count * sizeof(Vector3));
  cur_bytes = cur_bytes.subspan(vert_count * sizeof(Vector3));

  mesh_data.uvs.resize(vert_count);
  std::memcpy(mesh_data.uvs.data(), cur_bytes.data(), vert_count * sizeof(Vector2));
  cur_bytes = cur_bytes.subspan(vert_count * sizeof(Vector2));

  mesh_data.bone_weights.resize(vert_count);
  std::memcpy(mesh_data.bone_weights.data(), cur_bytes.data(), vert_count * sizeof(Vector4));
  cur_bytes = cur_bytes.subspan(vert_count * sizeof(Vector4));

  mesh_data.bone_indices.resize(vert_count);
  std::memcpy(mesh_data.bone_indices.data(), cur_bytes.data(),
    vert_count * sizeof(Vector<std::uint32_t, 4>));
  cur_bytes = cur_bytes.subspan(vert_count * sizeof(Vector<std::uint32_t, 4>));

  mesh_data.meshlets.resize(meshlet_count);
  std::memcpy(mesh_data.meshlets.data(), cur_bytes.data(), meshlet_count * sizeof(MeshletData));
  cur_bytes = cur_bytes.subspan(meshlet_count * sizeof(MeshletData));

  mesh_data.vertex_indices.resize(vtx_idx_count);
  std::memcpy(mesh_data.vertex_indices.data(), cur_bytes.data(), vtx_idx_count);
  cur_bytes = cur_bytes.subspan(vtx_idx_count);

  mesh_data.triangle_indices.resize(prim_idx_count);
  std::memcpy(mesh_data.triangle_indices.data(), cur_bytes.data(),
    prim_idx_count * sizeof(MeshletTriangleData));
  cur_bytes = cur_bytes.subspan(prim_idx_count * sizeof(MeshletTriangleData));

  mesh_data.cull_data.resize(meshlet_count);
  std::memcpy(mesh_data.cull_data.data(), cur_bytes.data(), meshlet_count * sizeof(MeshletCullData));
  cur_bytes = cur_bytes.subspan(meshlet_count * sizeof(MeshletCullData));

  // Material slots

  mesh_data.material_slots.resize(material_slot_count);

  for (auto i{0ull}; i < material_slot_count; i++) {
    if (!DeserializeFromBinary(cur_bytes, mesh_data.material_slots[i].name)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(mesh_data.material_slots[i].name.size() + 8);
  }

  // Submeshes

  mesh_data.submeshes.resize(submesh_count);

  for (auto i{0ull}; i < submesh_count; i++) {
    if (!DeserializeFromBinary(cur_bytes, mesh_data.submeshes[i].first_meshlet)) {
      return nullptr;
    }
    cur_bytes = cur_bytes.subspan(sizeof(std::uint32_t));

    if (!DeserializeFromBinary(cur_bytes, mesh_data.submeshes[i].meshlet_count)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof(std::uint32_t));

    if (!DeserializeFromBinary(cur_bytes, mesh_data.submeshes[i].base_vertex)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof(std::uint32_t));

    if (!DeserializeFromBinary(cur_bytes, mesh_data.submeshes[i].material_idx)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof(std::uint32_t));

    for (auto j{0}; j < 3; j++) {
      if (!DeserializeFromBinary(cur_bytes, mesh_data.submeshes[i].bounds.min[j])) {
        return nullptr;
      }

      cur_bytes = cur_bytes.subspan(sizeof(float));
    }

    for (auto j{0}; j < 3; j++) {
      if (!DeserializeFromBinary(cur_bytes, mesh_data.submeshes[i].bounds.max[j])) {
        return nullptr;
      }

      cur_bytes = cur_bytes.subspan(sizeof(float));
    }
  }

  // Animations

  mesh_data.animations.resize(anim_count);

  for (auto i{0ull}; i < anim_count; i++) {
    if (!DeserializeFromBinary(cur_bytes, mesh_data.animations[i].name)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(mesh_data.animations[i].name.size() + 8);

    if (!DeserializeFromBinary(cur_bytes, mesh_data.animations[i].duration)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof(float));

    if (!DeserializeFromBinary(cur_bytes, mesh_data.animations[i].ticks_per_second)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof(float));
    std::uint64_t node_anim_count;

    if (!DeserializeFromBinary(cur_bytes, node_anim_count)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof node_anim_count);
    mesh_data.animations[i].node_anims.resize(node_anim_count);

    for (auto j{0ull}; j < node_anim_count; j++) {
      if (!DeserializeFromBinary(cur_bytes, mesh_data.animations[i].node_anims[j].node_idx)) {
        return nullptr;
      }

      cur_bytes = cur_bytes.subspan(sizeof(std::uint32_t));
      std::uint64_t pos_key_count;

      if (!DeserializeFromBinary(cur_bytes, pos_key_count)) {
        return nullptr;
      }

      cur_bytes = cur_bytes.subspan(sizeof pos_key_count);
      std::uint64_t rot_key_count;

      if (!DeserializeFromBinary(cur_bytes, rot_key_count)) {
        return nullptr;
      }

      cur_bytes = cur_bytes.subspan(sizeof rot_key_count);
      std::uint64_t scale_key_count;

      if (!DeserializeFromBinary(cur_bytes, scale_key_count)) {
        return nullptr;
      }

      cur_bytes = cur_bytes.subspan(sizeof scale_key_count);

      mesh_data.animations[i].node_anims[j].position_keys.resize(pos_key_count);
      std::memcpy(mesh_data.animations[i].node_anims[j].position_keys.data(), cur_bytes.data(),
        pos_key_count * sizeof(AnimPositionKey));
      cur_bytes = cur_bytes.subspan(pos_key_count * sizeof(AnimPositionKey));


      mesh_data.animations[i].node_anims[j].rotation_keys.resize(rot_key_count);
      std::memcpy(mesh_data.animations[i].node_anims[j].rotation_keys.data(), cur_bytes.data(),
        rot_key_count * sizeof(AnimRotationKey));
      cur_bytes = cur_bytes.subspan(rot_key_count * sizeof(AnimRotationKey));

      mesh_data.animations[i].node_anims[j].scaling_keys.resize(scale_key_count);
      std::memcpy(mesh_data.animations[i].node_anims[j].scaling_keys.data(), cur_bytes.data(),
        scale_key_count * sizeof(AnimScalingKey));
      cur_bytes = cur_bytes.subspan(scale_key_count * sizeof(AnimScalingKey));
    }
  }

  // Skeleton nodes

  mesh_data.skeleton.resize(skeleton_size);

  for (auto i{0ull}; i < skeleton_size; i++) {
    if (!DeserializeFromBinary(cur_bytes, mesh_data.skeleton[i].name)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(mesh_data.skeleton[i].name.size() + 8);
    bool has_parent;

    if (!DeserializeFromBinary(cur_bytes, has_parent)) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof has_parent);

    if (has_parent) {
      std::uint32_t parent_idx;

      if (!DeserializeFromBinary(cur_bytes, parent_idx)) {
        return nullptr;
      }

      mesh_data.skeleton[i].parent_idx = parent_idx;
      cur_bytes = cur_bytes.subspan(sizeof std::uint32_t);
    }

    std::memcpy(mesh_data.skeleton[i].transform.GetData(), cur_bytes.data(), sizeof(Matrix4));
    cur_bytes = cur_bytes.subspan(sizeof Matrix4);
  }

  // Bones

  mesh_data.bones.resize(bone_count);
  std::memcpy(mesh_data.bones.data(), cur_bytes.data(), bone_count * sizeof(Bone));
  cur_bytes = cur_bytes.subspan(bone_count * sizeof(Bone));

  // Bounds

  for (auto j{0}; j < 3; j++) {
    if (!DeserializeFromBinary(cur_bytes, mesh_data.bounds.min[j])) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof(float));
  }

  for (auto j{0}; j < 3; j++) {
    if (!DeserializeFromBinary(cur_bytes, mesh_data.bounds.max[j])) {
      return nullptr;
    }

    cur_bytes = cur_bytes.subspan(sizeof(float));
  }

  // Index format

  if (!DeserializeFromBinary(cur_bytes, mesh_data.idx32)) {
    return nullptr;
  }

  cur_bytes = cur_bytes.subspan(sizeof mesh_data.idx32);

  assert(cur_bytes.empty());

  return std::make_unique<Mesh>(std::move(mesh_data), CpuResidencyPolicy::kReleaseAfterUpload);
}


auto AssetManager::LoadMaterial(
  std::span<std::byte const> const bytes,
  YamlDeserializeContext const& ctx
) -> MaybeNull<std::unique_ptr<Asset>> {
  // TODO rewrite this to spanstream when upgrading to C++23
  auto mtl = std::make_unique<Material>();
  mtl->Deserialize(YAML::Load(std::string{
    reinterpret_cast<char const*>(bytes.data()), bytes.size()
  }), ctx);
  return mtl;
}


auto AssetManager::LoadPrefab(
  std::span<std::byte const> const bytes,
  YamlDeserializeContext const& ctx
) -> MaybeNull<std::unique_ptr<Asset>> {
  // TODO rewrite this to spanstream when upgrading to C++23
  auto prefab = std::make_unique<Prefab>();
  prefab->Deserialize(YAML::Load(std::string{
    reinterpret_cast<char const*>(bytes.data()), bytes.size()
  }), ctx);
  return prefab;
}
}
