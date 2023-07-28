#include "Mesh.hpp"

#include "../Renderer.hpp"
#include "../Util.hpp"
#include "../Serialization.hpp"

#include <utility>
#include <format>
#include <imgui.h>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Mesh>{ "Mesh" };
}


namespace sorcery {
auto Mesh::UploadToGPU() -> void {
  D3D11_BUFFER_DESC const posDesc{
    .ByteWidth = clamp_cast<UINT>(mCPUData->positions.size() * sizeof(Vector3)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const posBufData{
    .pSysMem = mCPUData->positions.data()
  };

  if (FAILED(gRenderer.GetDevice()->CreateBuffer(&posDesc, &posBufData, mPosBuf.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create mesh position buffer." };
  }

  D3D11_BUFFER_DESC const normDesc{
    .ByteWidth = clamp_cast<UINT>(mCPUData->normals.size() * sizeof(Vector3)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const normBufData{
    .pSysMem = mCPUData->normals.data()
  };

  if (FAILED(gRenderer.GetDevice()->CreateBuffer(&normDesc, &normBufData, mNormBuf.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create mesh normal buffer." };
  }

  D3D11_BUFFER_DESC const uvDesc{
    .ByteWidth = clamp_cast<UINT>(mCPUData->uvs.size() * sizeof(Vector2)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const uvBufData{
    .pSysMem = mCPUData->uvs.data()
  };

  if (FAILED(gRenderer.GetDevice()->CreateBuffer(&uvDesc, &uvBufData, mUvBuf.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create mesh uv buffer." };
  }

  D3D11_BUFFER_DESC const indDesc{
    .ByteWidth = clamp_cast<UINT>(mCPUData->indices.size() * sizeof(u32)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_INDEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const indBufData{
    .pSysMem = mCPUData->indices.data()
  };

  if (FAILED(gRenderer.GetDevice()->CreateBuffer(&indDesc, &indBufData, mIndBuf.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create mesh index buffer." };
  }

  D3D11_BUFFER_DESC const tangentBufDesc{
    .ByteWidth = static_cast<UINT>(mCPUData->tangents.size() * sizeof(Vector3)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const tangentBufData{
    .pSysMem = mCPUData->tangents.data()
  };

  if (FAILED(gRenderer.GetDevice()->CreateBuffer(&tangentBufDesc, &tangentBufData, mTangentBuf.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create mesh tangent buffer." };
  }
}


auto Mesh::CalculateBounds() -> void {
  mBounds = {};

  for (auto const& position : mCPUData->positions) {
    mBounds.min = Vector3{ std::min(mBounds.min[0], position[0]), std::min(mBounds.min[1], position[1]), std::min(mBounds.min[2], position[2]) };
    mBounds.max = Vector3{ std::max(mBounds.max[0], position[0]), std::max(mBounds.max[1], position[1]), std::max(mBounds.max[2], position[2]) };
  }
}


Mesh::Mesh(Data data, bool const keepDataInCPUMemory) {
  SetData(std::move(data));
  ValidateAndUpdate(keepDataInCPUMemory);
}


auto Mesh::GetPositions() const noexcept -> std::span<Vector3 const> {
  return mCPUData
           ? mCPUData->positions
           : std::span<Vector3 const>{};
}


auto Mesh::SetPositions(std::vector<Vector3> positions, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  if (!mCPUData) {
    if (!allocateCPUMemoryIfNeeded) {
      return;
    }

    mCPUData = new GeometryData{};
  }

  mCPUData->positions = std::move(positions);
}


auto Mesh::GetNormals() const noexcept -> std::span<Vector3 const> {
  return mCPUData
           ? mCPUData->normals
           : std::span<Vector3 const>{};
}


auto Mesh::SetNormals(std::vector<Vector3> normals, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  if (!mCPUData) {
    if (!allocateCPUMemoryIfNeeded) {
      return;
    }

    mCPUData = new GeometryData{};
  }

  mCPUData->normals = std::move(normals);
}


auto Mesh::GetUVs() const noexcept -> std::span<Vector2 const> {
  return mCPUData
           ? mCPUData->uvs
           : std::span<Vector2 const>{};
}


auto Mesh::SetUVs(std::vector<Vector2> uvs, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  if (!mCPUData) {
    if (!allocateCPUMemoryIfNeeded) {
      return;
    }

    mCPUData = new GeometryData{};
  }

  mCPUData->uvs = std::move(uvs);
}


auto Mesh::GetTangents() const noexcept -> std::span<Vector3 const> {
  return mCPUData
           ? mCPUData->tangents
           : std::span<Vector3 const>{};
}


auto Mesh::SetTangents(std::vector<Vector3> tangents, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  if (!mCPUData) {
    if (!allocateCPUMemoryIfNeeded) {
      return;
    }

    mCPUData = new GeometryData{};
  }

  mCPUData->tangents = std::move(tangents);
}


auto Mesh::GetIndices() const noexcept -> std::span<u32 const> {
  return mCPUData
           ? mCPUData->indices
           : std::span<u32 const>{};
}


auto Mesh::SetIndices(std::vector<u32> indices, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  if (!mCPUData) {
    if (!allocateCPUMemoryIfNeeded) {
      return;
    }

    mCPUData = new GeometryData{};
  }

  mCPUData->indices = std::move(indices);
}


auto Mesh::GetSubMeshes() const noexcept -> std::span<SubMeshData const> {
  return mSubmeshes;
}


auto Mesh::SetSubMeshes(std::vector<SubMeshData> subMeshes, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  mSubmeshes = std::move(subMeshes);
}


auto Mesh::GetBounds() const noexcept -> AABB const& {
  return mBounds;
}


auto Mesh::ValidateAndUpdate(bool const keepDataInCPUMemory) -> void {
  if (!mCPUData) {
    return;
  }

  auto constexpr errFmt{ "Failed to validate mesh {} (\"{}\"). {}." };

  if (mCPUData->positions.size() != mCPUData->normals.size() ||
      mCPUData->normals.size() != mCPUData->uvs.size() ||
      mCPUData->uvs.size() != mCPUData->tangents.size() ||
      mCPUData->positions.empty() || mCPUData->indices.empty()) {
    throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "Inconsistent number of positions, normals, UVs and tangents.") };
  }

  for (auto const& [baseVertex, firstIndex, indexCount, mtlSlotName] : mSubmeshes) {
    if (baseVertex >= std::ssize(mCPUData->positions)) {
      throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains a base vertex greater than the number of vertices.") };
    }

    if (firstIndex >= std::ssize(mCPUData->indices)) {
      throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains a first index greater than the number of indices.") };
    }

    if (firstIndex + indexCount > std::ssize(mCPUData->indices)) {
      throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains an out of bounds index region.") };
    }

    for (int i = firstIndex; i < firstIndex + indexCount; i++) {
      if (mCPUData->indices[i] + baseVertex > mCPUData->positions.size()) {
        throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains an out of bounds vertex index.") };
      }
    }
  }

  mVertexCount = static_cast<int>(std::ssize(mCPUData->positions));
  mIndexCount = static_cast<int>(std::ssize(mCPUData->indices));
  mSubmeshCount = static_cast<int>(std::ssize(mSubmeshes));

  CalculateBounds();
  UploadToGPU();

  if (!keepDataInCPUMemory) {
    ReleaseCPUMemory();
  }
}


auto Mesh::GetPositionBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
  return mPosBuf;
}


auto Mesh::GetNormalBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
  return mNormBuf;
}


auto Mesh::GetUVBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
  return mUvBuf;
}


auto Mesh::GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
  return mIndBuf;
}


auto Mesh::GetTangentBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
  return mTangentBuf;
}


auto Mesh::SetData(Data data, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  if (!mCPUData) {
    if (!allocateCPUMemoryIfNeeded) {
      return;
    }

    mCPUData = new GeometryData{};
  } else {
    mCPUData->positions = std::move(data.positions);
    mCPUData->normals = std::move(data.normals);
    mCPUData->uvs = std::move(data.uvs);
    mCPUData->tangents = std::move(data.tangents);
    mCPUData->indices = std::move(data.indices);
    mSubmeshes = std::move(data.subMeshes);
  }
}


auto Mesh::ReleaseCPUMemory() -> void {
  delete mCPUData;
  mCPUData = nullptr;
}


auto Mesh::HasCPUMemory() const noexcept -> bool {
  return mCPUData != nullptr;
}


auto Mesh::GetVertexCount() const noexcept -> int {
  return mVertexCount;
}


auto Mesh::GetIndexCount() const noexcept -> int {
  return mIndexCount;
}


auto Mesh::GetSubmeshCount() const noexcept -> int {
  return mSubmeshCount;
}


auto Mesh::OnDrawProperties() -> void {
  Resource::OnDrawProperties();

  if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", "Vertex Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(GetVertexCount()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Index Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(GetIndexCount()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Submesh Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(GetSubmeshCount()).c_str());

    ImGui::EndTable();
  }
}
}
