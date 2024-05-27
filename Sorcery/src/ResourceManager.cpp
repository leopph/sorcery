#include "ResourceManager.hpp"
#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "ExternalResource.hpp"
#include "FileIo.hpp"
#include "engine_context.hpp"
#include "job_system.hpp"
#include "rendering/render_manager.hpp"
#include "Resources/Scene.hpp"

#include <DirectXTex.h>
#include <wrl/client.h>

#include <cassert>
#include <iostream>
#include <ranges>
#include <utility>

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


auto ResourceManager::ResourceGuidLess::operator()(std::unique_ptr<Resource> const& lhs,
                                                   std::unique_ptr<Resource> const& rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs->GetGuid();
}


auto ResourceManager::ResourceGuidLess::operator()(std::unique_ptr<Resource> const& lhs,
                                                   Guid const& rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs;
}


auto ResourceManager::ResourceGuidLess::operator()(Guid const& lhs,
                                                   std::unique_ptr<Resource> const& rhs) const noexcept -> bool {
  return lhs < rhs->GetGuid();
}


auto ResourceManager::InternalLoadResource(Guid const& guid, ResourceDescription const& desc) -> ObserverPtr<Resource> {
  Job* loader_job;

  std::cout << "loading " << desc.name << '\n';

  {
    auto loader_jobs{loader_jobs_.Lock()};

    if (auto const it{loader_jobs->find(guid)}; it != loader_jobs->end()) {
      loader_job = it->second;
    } else {
      struct LoaderJobData {
        Guid const* guid;
        ResourceDescription const* desc;
        decltype(loaded_resources_)* resources;
      };

      loader_job = job_system_->CreateJob([](void const* const data_ptr) {
        auto const& job_data{*static_cast<LoaderJobData const*>(data_ptr)};

        std::unique_ptr<Resource> res;

        if (job_data.desc->pathAbs.extension() == EXTERNAL_RESOURCE_EXT) {
          std::vector<std::uint8_t> fileBytes;

          if (!ReadFileBinary(job_data.desc->pathAbs, fileBytes)) {
            return;
          }

          ExternalResourceCategory resCat;
          std::vector<std::byte> resBytes;

          if (!UnpackExternalResource(as_bytes(std::span{fileBytes}), resCat, resBytes)) {
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
        } else if (job_data.desc->pathAbs.extension() == SCENE_RESOURCE_EXT) {
          res = CreateDeserialize<Scene>(YAML::LoadFile(job_data.desc->pathAbs.string()));
        } else if (job_data.desc->pathAbs.extension() == MATERIAL_RESOURCE_EXT) {
          res = CreateDeserialize<Material>(YAML::LoadFile(job_data.desc->pathAbs.string()));
        }

        if (res) {
          res->SetGuid(*job_data.guid);
          res->SetName(job_data.desc->name);

          auto const [it, inserted]{job_data.resources->Lock()->emplace(std::move(res))};
          assert(inserted);
        }
      }, LoaderJobData{&guid, &desc, &loaded_resources_});
      job_system_->Run(loader_job);
      loader_jobs->emplace(guid, loader_job);
    }
  }

  assert(loader_job);
  job_system_->Wait(loader_job);

  {
    loader_jobs_.Lock()->erase(guid);
  }

  {
    auto const resources{loaded_resources_.LockShared()};
    auto const it{resources->find(guid)};
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

  auto tex{g_engine_context.render_manager->CreateReadOnlyTexture(img)};

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
  auto curBytes{as_bytes(std::span{bytes})};
  std::uint64_t vertexCount;

  if (!DeserializeFromBinary(curBytes, vertexCount)) {
    return nullptr;
  }

  curBytes = curBytes.subspan(sizeof vertexCount);
  std::uint64_t idxCount;

  if (!DeserializeFromBinary(curBytes, idxCount)) {
    return nullptr;
  }

  curBytes = curBytes.subspan(sizeof idxCount);
  std::uint64_t mtlCount;

  if (!DeserializeFromBinary(curBytes, mtlCount)) {
    return nullptr;
  }

  curBytes = curBytes.subspan(sizeof mtlCount);
  std::uint64_t submeshCount;

  if (!DeserializeFromBinary(curBytes, submeshCount)) {
    return nullptr;
  }

  curBytes = curBytes.subspan(sizeof submeshCount);
  std::int32_t idx32;

  if (!DeserializeFromBinary(curBytes, idx32)) {
    return nullptr;
  }

  curBytes = curBytes.subspan(sizeof idx32);

  Mesh::Data meshData;

  meshData.positions.resize(vertexCount);
  std::memcpy(meshData.positions.data(), curBytes.data(), vertexCount * sizeof(Vector3));
  curBytes = curBytes.subspan(vertexCount * sizeof(Vector3));

  meshData.normals.resize(vertexCount);
  std::memcpy(meshData.normals.data(), curBytes.data(), vertexCount * sizeof(Vector3));
  curBytes = curBytes.subspan(vertexCount * sizeof(Vector3));

  meshData.uvs.resize(vertexCount);
  std::memcpy(meshData.uvs.data(), curBytes.data(), vertexCount * sizeof(Vector2));
  curBytes = curBytes.subspan(vertexCount * sizeof(Vector2));

  meshData.tangents.resize(vertexCount);
  std::memcpy(meshData.tangents.data(), curBytes.data(), vertexCount * sizeof(Vector3));
  curBytes = curBytes.subspan(vertexCount * sizeof(Vector3));

  if (idx32) {
    meshData.indices.emplace<std::vector<std::uint32_t>>();
  }

  std::visit([idxCount, &curBytes]<typename T>(std::vector<T>& indices) {
    indices.resize(idxCount);
    std::memcpy(indices.data(), curBytes.data(), idxCount * sizeof(T));
    curBytes = curBytes.subspan(idxCount * sizeof(T));
  }, meshData.indices);

  meshData.material_slots.resize(mtlCount);

  for (auto i{0ull}; i < mtlCount; i++) {
    if (!DeserializeFromBinary(curBytes, meshData.material_slots[i].name)) {
      return nullptr;
    }

    curBytes = curBytes.subspan(meshData.material_slots[i].name.size() + 8);
  }

  meshData.sub_meshes.resize(submeshCount);

  for (auto i{0ull}; i < submeshCount; i++) {
    if (!DeserializeFromBinary(curBytes, meshData.sub_meshes[i].base_vertex)) {
      return nullptr;
    }

    curBytes = curBytes.subspan(sizeof(int));

    if (!DeserializeFromBinary(curBytes, meshData.sub_meshes[i].first_index)) {
      return nullptr;
    }

    curBytes = curBytes.subspan(sizeof(int));

    if (!DeserializeFromBinary(curBytes, meshData.sub_meshes[i].index_count)) {
      return nullptr;
    }

    curBytes = curBytes.subspan(sizeof(int));

    if (!DeserializeFromBinary(curBytes, meshData.sub_meshes[i].material_index)) {
      return nullptr;
    }

    curBytes = curBytes.subspan(sizeof(int));
  }

  return Create<Mesh>(std::move(meshData));
}


ResourceManager::ResourceManager(JobSystem& job_system) :
  job_system_{&job_system} {
  default_mtl_ = Create<Material>();
  default_mtl_->SetGuid(default_mtl_guid_);
  default_mtl_->SetName("Default Material");
  default_resources_.emplace_back(default_mtl_.get());

  std::vector<Vector3> cubeNormals;
  CalculateNormals(kCubePositions, kCubeIndices, cubeNormals);

  std::vector<Vector3> cubeTangents;
  CalculateTangents(kCubePositions, kCubeUvs, kCubeIndices, cubeTangents);

  std::vector<Vector3> quadNormals;
  CalculateNormals(kQuadPositions, kQuadIndices, quadNormals);

  std::vector<Vector3> quadTangents;
  CalculateTangents(kQuadPositions, kQuadUvs, kQuadIndices, quadTangents);

  cube_mesh_ = Create<Mesh>();
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
  default_resources_.emplace_back(cube_mesh_.get());

  plane_mesh_ = Create<Mesh>();
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
  default_resources_.emplace_back(plane_mesh_.get());

  sphere_mesh_ = Create<Mesh>();
  sphere_mesh_->SetGuid(sphere_mesh_guid_);
  sphere_mesh_->SetName("Sphere");
  std::vector<Vector3> spherePositions;
  std::vector<Vector3> sphereNormals;
  std::vector<Vector3> sphereTangents;
  std::vector<Vector2> sphereUvs;
  std::vector<std::uint32_t> sphereIndices;
  rendering::GenerateSphereMesh(1, 50, 50, spherePositions, sphereNormals, sphereUvs, sphereIndices);
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
  default_resources_.emplace_back(sphere_mesh_.get());
}


auto ResourceManager::Unload(Guid const& guid) -> void {
  auto resources{loaded_resources_.Lock()};

  if (auto const it{resources->find(guid)}; it != std::end(*resources)) {
    resources->erase(it);
  }
}


auto ResourceManager::UnloadAll() -> void {
  loaded_resources_.Lock()->clear();
}


auto ResourceManager::IsLoaded(Guid const& guid) -> bool {
  for (auto const& res : default_resources_) {
    if (res->GetGuid() == guid) {
      return true;
    }
  }

  return loaded_resources_.LockShared()->contains(guid);
}


auto ResourceManager::UpdateMappings(std::map<Guid, ResourceDescription> mappings) -> void {
  while (true) {
    if (auto self_mappings{mappings_.TryLock()}) {
      **self_mappings = std::move(mappings);
      break;
    }
  }
}


auto ResourceManager::GetGuidsForResourcesOfType(rttr::type const& type,
                                                 std::vector<Guid>& out) noexcept -> void {
  // Default resources
  for (auto const& res : default_resources_) {
    if (rttr::type::get(*res).is_derived_from(type)) {
      out.emplace_back(res->GetGuid());
    }
  }

  // File mappings
  for (auto const& [guid, desc] : *mappings_.LockShared()) {
    if (desc.type.is_derived_from(type)) {
      out.emplace_back(guid);
    }
  }

  // Other, loaded resources that don't come from files
  for (auto const& res : *loaded_resources_.LockShared()) {
    auto contains{false};
    for (auto const& guid : out) {
      if (guid == res->GetGuid()) {
        contains = true;
        break;
      }
    }

    if (!contains && rttr::type::get(*res).is_derived_from(type)) {
      out.emplace_back(res->GetGuid());
    }
  }
}


auto ResourceManager::GetInfoForResourcesOfType(rttr::type const& type, std::vector<ResourceInfo>& out) -> void {
  // Default resources
  for (auto const& res : default_resources_) {
    if (auto const res_type{rttr::type::get(*res)}; res_type.is_derived_from(type)) {
      out.emplace_back(res->GetGuid(), res->GetName(), res_type);
    }
  }

  // File mappings
  for (auto const& [guid, desc] : *mappings_.LockShared()) {
    if (desc.type.is_derived_from(type)) {
      out.emplace_back(guid, desc.name, desc.type);
    }
  }

  // Other, loaded resources that don't come from files
  for (auto const& res : *loaded_resources_.LockShared()) {
    auto contains{false};
    for (auto const& res_info : out) {
      if (res_info.guid == res->GetGuid()) {
        contains = true;
        break;
      }
    }

    if (!contains && rttr::type::get(*res).is_derived_from(type)) {
      out.emplace_back(res->GetGuid(), res->GetName(), res->get_type());
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
}
