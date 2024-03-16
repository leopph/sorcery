#include "ResourceManager.hpp"
#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "ExternalResource.hpp"
#include "FileIo.hpp"
#include "engine_context.hpp"
#include "Resources/Scene.hpp"

#include <DirectXTex.h>
#include <wrl/client.h>

#include <cassert>
#include <ranges>

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


auto ResourceManager::ResourceGuidLess::operator()(Resource* const lhs, Resource* const rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs->GetGuid();
}


auto ResourceManager::ResourceGuidLess::operator()(Resource* const lhs, Guid const& rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs;
}


auto ResourceManager::ResourceGuidLess::operator()(Guid const& lhs, Resource* const rhs) const noexcept -> bool {
  return lhs < rhs->GetGuid();
}


auto ResourceManager::InternalLoadResource(Guid const& guid, ResourceDescription const& desc) -> Resource* {
  Resource* res{nullptr};

  if (desc.pathAbs.extension() == EXTERNAL_RESOURCE_EXT) {
    std::vector<std::uint8_t> fileBytes;

    if (!ReadFileBinary(desc.pathAbs, fileBytes)) {
      return nullptr;
    }

    ExternalResourceCategory resCat;
    std::vector<std::byte> resBytes;

    if (!UnpackExternalResource(as_bytes(std::span{fileBytes}), resCat, resBytes)) {
      return nullptr;
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
  } else if (desc.pathAbs.extension() == SCENE_RESOURCE_EXT) {
    auto const scene{CreateAndInitialize<Scene>()};
    scene->Deserialize(YAML::LoadFile(desc.pathAbs.string()));
    res = scene;
  } else if (desc.pathAbs.extension() == MATERIAL_RESOURCE_EXT) {
    auto const mtl{CreateAndInitialize<Material>()};
    mtl->Deserialize(YAML::LoadFile(desc.pathAbs.string()));
    res = mtl;
  }

  if (res) {
    res->SetGuid(guid);
    res->SetName(desc.name);

    auto const [it, inserted]{mResources.emplace(res)};
    assert(inserted);
  }

  return res;
}


auto ResourceManager::LoadTexture(std::span<std::byte const> const bytes) noexcept -> MaybeNull<Resource*> {
  DirectX::TexMetadata meta;
  DirectX::ScratchImage img;
  if (FAILED(LoadFromDDSMemory(bytes.data(), bytes.size(), DirectX::DDS_FLAGS_NONE, &meta, img))) {
    return nullptr;
  }

  auto tex{g_engine_context.render_manager->CreateReadOnlyTexture(img)};

  if (!tex) {
    return nullptr;
  }

  Resource* ret;

  if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D) {
    if (meta.IsCubemap()) {
      ret = new Cubemap{std::move(tex)};
    } else {
      ret = new Texture2D{std::move(tex)};
    }
  } else {
    return nullptr;
  }

  ret->OnInit();
  return ret;
}


auto ResourceManager::LoadMesh(std::span<std::byte const> const bytes) -> MaybeNull<Resource*> {
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

  auto const ret{new Mesh{std::move(meshData)}};
  ret->OnInit();
  return ret;
}


ResourceManager::ResourceManager() {
  default_mtl_.Reset(CreateAndInitialize<Material>());
  default_mtl_->SetGuid(default_material_guid_);
  default_mtl_->SetName("Default Material");
  Add(default_mtl_.Get());

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
  Add(cube_mesh_.Get());

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
  Add(plane_mesh_.Get());

  sphere_mesh_.Reset(CreateAndInitialize<Mesh>());
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
  Add(sphere_mesh_.Get());
}


auto ResourceManager::Unload(Guid const& guid) -> void {
  if (auto const it{mResources.find(guid)}; it != std::end(mResources)) {
    Destroy(**it);
    mResources.erase(it);
  }
}


auto ResourceManager::IsLoaded(Guid const& guid) const -> bool {
  return mResources.contains(guid);
}


auto ResourceManager::UpdateMappings(std::map<Guid, ResourceDescription> mappings) -> void {
  mMappings = std::move(mappings);
}


auto ResourceManager::GetGuidsForResourcesOfType(rttr::type const& type,
                                                 std::vector<Guid>& out) const noexcept -> void {
  for (auto const& [guid, desc] : mMappings) {
    if (desc.type.is_derived_from(type)) {
      out.emplace_back(guid);
    }
  }

  // Resources that don't come from files
  for (auto const res : mResources) {
    auto contains{false};
    for (auto const& guid : out) {
      if (guid <=> res->GetGuid() == std::strong_ordering::equal) {
        contains = true;
        break;
      }
    }

    if (!contains && rttr::type::get(*res).is_derived_from(type)) {
      out.emplace_back(res->GetGuid());
    }
  }
}


auto ResourceManager::GetDefaultMaterial() const noexcept -> ObserverPtr<Material> {
  return default_mtl_;
}


auto ResourceManager::GetCubeMesh() const noexcept -> ObserverPtr<Mesh> {
  return cube_mesh_;
}


auto ResourceManager::GetPlaneMesh() const noexcept -> ObserverPtr<Mesh> {
  return plane_mesh_;
}


auto ResourceManager::GetSphereMesh() const noexcept -> ObserverPtr<Mesh> {
  return sphere_mesh_;
}
}
