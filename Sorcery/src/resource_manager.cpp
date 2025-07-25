#include "resource_manager.hpp"

#include <cassert>
#include <ranges>
#include <utility>

#include <DirectXTex.h>
#include <wrl/client.h>

#include "app.hpp"
#include "ExternalResource.hpp"
#include "FileIo.hpp"
#include "job_system.hpp"
#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "rendering/render_manager.hpp"
#include "Resources/Scene.hpp"

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


auto ResourceManager::ResourceIdLess::operator()(std::unique_ptr<Resource> const& lhs,
                                                 std::unique_ptr<Resource> const& rhs) const noexcept -> bool {
  return lhs->GetId() < rhs->GetId();
}


auto ResourceManager::ResourceIdLess::operator()(std::unique_ptr<Resource> const& lhs,
                                                 ResourceId const& rhs) const noexcept -> bool {
  return lhs->GetId() < rhs;
}


auto ResourceManager::ResourceIdLess::operator()(ResourceId const& lhs,
                                                 std::unique_ptr<Resource> const& rhs) const noexcept -> bool {
  return lhs < rhs->GetId();
}


auto ResourceManager::InternalLoadResource(ResourceId const& res_id,
                                           ResourceDescription const& desc) -> ObserverPtr<Resource> {
  ObserverPtr<Job> loader_job;

  struct JobData {
    ResourceId const* res_id;
    ResourceDescription const* desc;
    std::filesystem::path path_abs;
  };

  JobData job_data;

  {
    auto loader_jobs{loader_jobs_.Lock()};

    if (auto const job_it{loader_jobs->find(res_id)}; job_it != loader_jobs->end()) {
      loader_job = job_it->second;
    } else {
      auto const file_mappings{file_mappings_.LockShared()};

      if (auto const file_it{file_mappings->find(res_id.GetGuid())}; file_it != file_mappings->end()) {
        job_data.res_id = &res_id;
        job_data.desc = &desc;
        job_data.path_abs = file_it->second;

        loader_job = job_system_->CreateJob([this, &job_data] {
          std::unique_ptr<Resource> res;

          if (job_data.path_abs.extension() == EXTERNAL_RESOURCE_EXT) {
            std::vector<std::uint8_t> file_bytes;

            if (!ReadFileBinary(job_data.path_abs, file_bytes)) {
              return;
            }

            ExternalResourceCategory resCat;
            std::vector<std::byte> resBytes;

            if (!UnpackExternalResource(as_bytes(std::span{file_bytes}), resCat, resBytes)) {
              return;
            }

            switch (resCat) {
              case ExternalResourceCategory::Texture: {
                res = LoadTexture(resBytes);
                break;
              }

              case ExternalResourceCategory::Mesh: {
                res = LoadMesh(resBytes);
                break;
              }
            }
          } else if (job_data.path_abs.extension() == SCENE_RESOURCE_EXT) {
            res = CreateDeserialize<Scene>(YAML::LoadFile(job_data.path_abs.string()));
          } else if (job_data.path_abs.extension() == MATERIAL_RESOURCE_EXT) {
            res = CreateDeserialize<Material>(YAML::LoadFile(job_data.path_abs.string()));
          }

          if (res) {
            res->SetId(*job_data.res_id);
            res->SetName(job_data.desc->name);

            auto const [it, inserted]{loaded_resources_.Lock()->emplace(std::move(res))};
            assert(inserted);
          }
        });
        job_system_->Run(loader_job);
        loader_jobs->emplace(res_id, loader_job);
      }
    }
  }

  assert(loader_job);
  job_system_->Wait(loader_job);

  {
    loader_jobs_.Lock()->erase(res_id);
  }

  {
    auto const resources{loaded_resources_.LockShared()};
    auto const it{resources->find(res_id)};
    return ObserverPtr{it != resources->end() ? it->get() : nullptr};
  }
}


auto ResourceManager::LoadTexture(
  std::span<std::byte const> const bytes) noexcept -> MaybeNull<std::unique_ptr<Resource>> {
  DirectX::TexMetadata meta;
  DirectX::ScratchImage img;
  if (FAILED(LoadFromDDSMemory(bytes.data(), bytes.size(), DirectX::DDS_FLAGS_NONE, &meta, img))) {
    return nullptr;
  }

  auto tex{App::Instance().GetRenderManager().CreateReadOnlyTexture(img)};

  if (!tex) {
    return nullptr;
  }

  std::unique_ptr<Resource> ret;

  if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D) {
    if (meta.IsCubemap()) {
      ret = Create<Cubemap>(std::move(tex));
    } else {
      ret = Create<Texture2D>(std::move(tex));
    }
  } else {
    return nullptr;
  }

  return ret;
}


auto ResourceManager::LoadMesh(std::span<std::byte const> const bytes) -> MaybeNull<std::unique_ptr<Resource>> {
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

  return Create<Mesh>(std::move(mesh_data));
}


ResourceManager::ResourceManager(JobSystem& job_system) :
  job_system_{&job_system} {}


auto ResourceManager::Unload(ResourceId const& res_id) -> void {
  auto resources{loaded_resources_.Lock()};

  if (auto const it{resources->find(res_id)}; it != std::end(*resources)) {
    resources->erase(it);
  }
}


auto ResourceManager::UnloadAll() -> void {
  loaded_resources_.Lock()->clear();
}


auto ResourceManager::IsLoaded(ResourceId const& res_id) -> bool {
  for (auto const& res : default_resources_) {
    if (res->GetId() == res_id) {
      return true;
    }
  }

  return loaded_resources_.LockShared()->contains(res_id);
}


auto ResourceManager::UpdateMappings(std::map<ResourceId, ResourceDescription> res_mappings,
                                     std::map<Guid, std::filesystem::path> file_mappings) -> void {
  while (true) {
    auto self_res_mappings{res_mappings_.TryLock()};
    auto self_file_mappings{file_mappings_.TryLock()};

    if (self_res_mappings && self_file_mappings) {
      **self_res_mappings = std::move(res_mappings);
      **self_file_mappings = std::move(file_mappings);
      break;
    }
  }
}


auto ResourceManager::GetInfoForResourcesOfType(rttr::type const& type, std::vector<ResourceInfo>& out) -> void {
  // Default resources
  for (auto const& res : default_resources_) {
    if (auto const res_type{rttr::type::get(*res)}; res_type.is_derived_from(type)) {
      out.emplace_back(res->GetId(), res->GetName(), res_type);
    }
  }

  // File mappings
  for (auto const& [guid, desc] : *res_mappings_.LockShared()) {
    if (desc.type.is_derived_from(type)) {
      out.emplace_back(guid, desc.name, desc.type);
    }
  }

  // Other, loaded resources that don't come from files
  for (auto const& res : *loaded_resources_.LockShared()) {
    auto contains{false};
    for (auto const& res_info : out) {
      if (res_info.id == res->GetId()) {
        contains = true;
        break;
      }
    }

    if (!contains && rttr::type::get(*res).is_derived_from(type)) {
      out.emplace_back(res->GetId(), res->GetName(), res->get_type());
    }
  }
}


auto ResourceManager::GetDefaultMaterial() const noexcept -> ObserverPtr<Material> {
  return ObserverPtr{default_mtl_.get()};
}


auto ResourceManager::GetCubeMesh() const noexcept -> ObserverPtr<Mesh> {
  return ObserverPtr{cube_mesh_.get()};
}


auto ResourceManager::GetPlaneMesh() const noexcept -> ObserverPtr<Mesh> {
  return ObserverPtr{plane_mesh_.get()};
}


auto ResourceManager::GetSphereMesh() const noexcept -> ObserverPtr<Mesh> {
  return ObserverPtr{sphere_mesh_.get()};
}


auto ResourceManager::CreateDefaultResources() -> void {
  if (!default_mtl_) {
    default_mtl_ = Create<Material>();
    default_mtl_->SetId({default_mtl_guid_, 0});
    default_mtl_->SetName("Default Material");
    default_resources_.emplace_back(default_mtl_.get());
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

    cube_mesh_ = Create<Mesh>(cube_data);
    cube_mesh_->SetId({cube_mesh_guid_, 0});
    cube_mesh_->SetName("Cube");
    default_resources_.emplace_back(cube_mesh_.get());
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

    plane_mesh_ = Create<Mesh>(plane_data);
    plane_mesh_->SetId({plane_mesh_guid_, 0});
    plane_mesh_->SetName("Plane");
    default_resources_.emplace_back(plane_mesh_.get());
  }

  if (!sphere_mesh_) {
    MeshData sphere_data;
    std::vector<std::uint32_t> sphere_indices;

    rendering::GenerateSphereMesh(1, 50, 50, sphere_data.positions, sphere_data.normals,
      sphere_data.uvs, sphere_indices);
    CalculateTangents(sphere_data.positions, sphere_data.uvs, sphere_indices,
      sphere_data.tangents);

    if (!ComputeMeshlets<std::uint32_t, Vector3>(sphere_indices, sphere_data.positions, sphere_data.meshlets,
      sphere_data.vertex_indices, sphere_data.triangle_indices, sphere_data.cull_data)) {
      throw std::runtime_error{"Failed to compute meshlets for default sphere mesh."};
    }

    sphere_data.material_slots.emplace_back("Material");
    sphere_data.submeshes.emplace_back(0, static_cast<std::uint32_t>(sphere_data.meshlets.size()), 0, 0,
      AABB::FromVertices(sphere_data.positions));
    sphere_data.bounds = sphere_data.submeshes[0].bounds;
    sphere_data.idx32 = true;

    sphere_mesh_ = Create<Mesh>(sphere_data);
    sphere_mesh_->SetId({sphere_mesh_guid_, 0});
    sphere_mesh_->SetName("Sphere");
    default_resources_.emplace_back(sphere_mesh_.get());
  }
}
}
