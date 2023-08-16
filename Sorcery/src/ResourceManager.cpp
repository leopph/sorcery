#include "ResourceManager.hpp"
#include "MemoryAllocation.hpp"
#include "Reflection.hpp"
#include "ExternalResource.hpp"
#include "FileIo.hpp"
#include "Renderer.hpp"

#include <directxtk/DDSTextureLoader.h>
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


auto ResourceManager::InternalLoadResource(Guid const& guid, std::filesystem::path const& resPathAbs) -> ObserverPtr<Resource> {
  ObserverPtr<Resource> res{nullptr};

  if (resPathAbs.extension() == EXTERNAL_RESOURCE_EXT) {
    std::vector<std::uint8_t> fileBytes;

    if (!ReadFileBinary(resPathAbs, fileBytes)) {
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
      }

      case ExternalResourceCategory::Mesh: {
        res = LoadMesh(resBytes);
      }
    }
  } else if (resPathAbs.extension() == SCENE_RESOURCE_EXT) { } else if (resPathAbs.extension() == MATERIAL_RESOURCE_EXT) { }

  if (res) {
    res->SetGuid(guid);
    res->SetName(resPathAbs.stem().string());

    auto const [it, inserted]{mResources.emplace(res)};
    assert(inserted);
  }

  return res;
}


auto ResourceManager::LoadTexture(std::span<std::byte const> const bytes) noexcept -> MaybeNull<ObserverPtr<Resource>> {
  ComPtr<ID3D11Resource> res;
  ComPtr<ID3D11ShaderResourceView> srv;

  if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(gRenderer.GetDevice(), reinterpret_cast<std::uint8_t const*>(bytes.data()), std::size(bytes), 0, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::DDS_LOADER_DEFAULT, res.GetAddressOf(), srv.GetAddressOf()))) {
    return nullptr;
  }

  D3D11_RESOURCE_DIMENSION resDim;
  res->GetType(&resDim);

  if (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    ComPtr<ID3D11Texture2D> tex2D;
    if (FAILED(res.As(&tex2D))) {
      return nullptr;
    }

    D3D11_TEXTURE2D_DESC desc;
    tex2D->GetDesc(&desc);

    if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) {
      return new Cubemap{*tex2D.Get(), *srv.Get()};
    }

    return new Texture2D{*tex2D.Get(), *srv.Get()};
  }

  return nullptr;
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

    meshData.subMeshes.resize(submeshCount);

    for (auto i{0ull}; i < submeshCount; i++) {
      if (!DeserializeFromBinary(curBytes, meshData.subMeshes[i].baseVertex)) {
        return nullptr;
      }

      curBytes = curBytes.subspan(sizeof(int));

      if (!DeserializeFromBinary(curBytes, meshData.subMeshes[i].firstIndex)) {
        return nullptr;
      }

      curBytes = curBytes.subspan(sizeof(int));

      if (!DeserializeFromBinary(curBytes, meshData.subMeshes[i].indexCount)) {
        return nullptr;
      }

      curBytes = curBytes.subspan(sizeof(int));

      if (!DeserializeFromBinary(curBytes, meshData.subMeshes[i].mtlSlotName)) {
        return nullptr;
      }

      curBytes = curBytes.subspan(meshData.subMeshes[i].mtlSlotName.size());
    }

    return new Mesh{std::move(meshData)};
  }
}


auto ResourceManager::Unload(Guid const& guid) -> void {
  if (auto const it{mResources.find(guid)}; it != std::end(mResources)) {
    delete *it;
    mResources.erase(it);
  }
}


auto ResourceManager::IsLoaded(Guid const& guid) const -> bool {
  return mResources.contains(guid);
}


auto ResourceManager::UpdateGuidPathMappings(std::map<Guid, std::filesystem::path> mappings) -> void {
  mGuidPathMappings = std::move(mappings);
}


auto ResourceManager::GetGuidsForResourcesOfType(rttr::type const& type, std::vector<Guid>& out) const noexcept -> void {
  /*for (auto const& resPathAbs : mGuidPathMappings | std::views::values) {
    Guid loadedGuid;
    std::unique_ptr<ResourceImporter> importer;

    if (LoadMeta(resPathAbs, std::addressof(loadedGuid), std::addressof(importer)) && importer->GetImportedType(resPathAbs).is_derived_from(type)) {
      out.emplace_back(loadedGuid);
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
  } TODO*/
}
}
