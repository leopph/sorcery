#include "ResourceManager.hpp"
#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "ExternalResource.hpp"
#include "FileIo.hpp"
#include "rendering\scene_renderer.hpp"
#include "Resources/Scene.hpp"

#include <DirectXTex.h>
#include <wrl/client.h>

#include <cassert>
#include <ranges>

using Microsoft::WRL::ComPtr;


namespace sorcery {
ResourceManager gResourceManager;


auto ResourceManager::ResourceGuidLess::operator()(ObserverPtr<Resource> const lhs, ObserverPtr<Resource> const rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs->GetGuid();
}


auto ResourceManager::ResourceGuidLess::operator()(ObserverPtr<Resource> const lhs, Guid const& rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs;
}


auto ResourceManager::ResourceGuidLess::operator()(Guid const& lhs, ObserverPtr<Resource> const rhs) const noexcept -> bool {
  return lhs < rhs->GetGuid();
}


auto ResourceManager::InternalLoadResource(Guid const& guid, ResourceDescription const& desc) -> ObserverPtr<Resource> {
  ObserverPtr<Resource> res{nullptr};

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


auto ResourceManager::LoadTexture(std::span<std::byte const> const bytes) noexcept -> MaybeNull<ObserverPtr<Resource>> {
  DirectX::TexMetadata meta;
  DirectX::ScratchImage img;
  if (FAILED(LoadFromDDSMemory(bytes.data(), bytes.size(), DirectX::DDS_FLAGS_NONE, &meta, img))) {
    return nullptr;
  }

  auto tex{gRenderer.LoadReadonlyTexture(img)};

  if (!tex) {
    return nullptr;
  }

  ObserverPtr<Resource> ret;

  if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D) {
    ret = new Texture2D{std::move(tex)};
  } else if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D && meta.IsCubemap()) {
    ret = new Cubemap{std::move(tex)};
  } else {
    return nullptr;
  }

  ret->OnInit();
  return ret;
}


auto ResourceManager::LoadMesh(std::span<std::byte const> const bytes) -> MaybeNull<ObserverPtr<Resource>> {
  if constexpr (std::endian::native != std::endian::little) {
    return nullptr; // TODO
  } else {
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


auto ResourceManager::GetGuidsForResourcesOfType(rttr::type const& type, std::vector<Guid>& out) const noexcept -> void {
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
}
