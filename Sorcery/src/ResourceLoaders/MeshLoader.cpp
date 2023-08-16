#include "MeshLoader.hpp"
#include "../FileIo.hpp"
#include "../Resources/Mesh.hpp"
#include "../Serialization.hpp"

#include <bit>
#include <cstdint>
#include <cstring>


namespace sorcery {
auto MeshLoader::Load(std::filesystem::path const& pathAbs) -> MaybeNull<ObserverPtr<Resource>> {
  if constexpr (std::endian::native != std::endian::little) {
    return nullptr; // TODO
  } else {
    std::vector<std::uint8_t> bytes;

    if (!ReadFileBinary(pathAbs, bytes)) {
      return nullptr;
    }

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
}
