#include "Mesh.hpp"

#include "../Renderer.hpp"
#include "../Util.hpp"
#include "../Serialization.hpp"

#include <imgui.h>

#include <cassert>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Mesh>{"Mesh"};
}


namespace sorcery {
auto Mesh::UploadToGpu() noexcept -> void {
  D3D11_BUFFER_DESC const posBufDesc{
    .ByteWidth = clamp_cast<UINT>(mCpuData->positions.size() * sizeof(Vector3)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const posBufData{
    .pSysMem = mCpuData->positions.data()
  };

  [[maybe_unused]] auto hr{gRenderer.GetDevice()->CreateBuffer(&posBufDesc, &posBufData, mPosBuf.ReleaseAndGetAddressOf())};
  assert(SUCCEEDED(hr));

  D3D11_BUFFER_DESC const normBufDesc{
    .ByteWidth = clamp_cast<UINT>(mCpuData->normals.size() * sizeof(Vector3)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const normBufData{
    .pSysMem = mCpuData->normals.data()
  };

  hr = gRenderer.GetDevice()->CreateBuffer(&normBufDesc, &normBufData, mNormBuf.ReleaseAndGetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_BUFFER_DESC const uvBufDesc{
    .ByteWidth = clamp_cast<UINT>(mCpuData->uvs.size() * sizeof(Vector2)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const uvBufData{
    .pSysMem = mCpuData->uvs.data()
  };

  hr = gRenderer.GetDevice()->CreateBuffer(&uvBufDesc, &uvBufData, mUvBuf.ReleaseAndGetAddressOf());
  assert(SUCCEEDED(hr));

  struct IdxBufInfo {
    UINT size;
    void* dataPtr;
  };

  auto const& [idxBufSize, idxBufDataPtr]{
    [this] {
      if (mIdxFormat == DXGI_FORMAT_R16_UINT) {
        return IdxBufInfo{.size = static_cast<UINT>(std::size(mCpuData->indices16) * sizeof(std::uint16_t)), .dataPtr = mCpuData->indices16.data()};
      }
      if (mIdxFormat == DXGI_FORMAT_R32_UINT) {
        return IdxBufInfo{.size = static_cast<UINT>(std::size(mCpuData->indices32) * sizeof(std::uint32_t)), .dataPtr = mCpuData->indices32.data()};
      }
      return IdxBufInfo{.size = 0, .dataPtr = nullptr};
    }()
  };

  D3D11_BUFFER_DESC const idxBufDesc{
    .ByteWidth = idxBufSize,
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_INDEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const idxBufData{
    .pSysMem = idxBufDataPtr
  };

  hr = gRenderer.GetDevice()->CreateBuffer(&idxBufDesc, &idxBufData, mIdxBuf.ReleaseAndGetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_BUFFER_DESC const tangentBufDesc{
    .ByteWidth = static_cast<UINT>(mCpuData->tangents.size() * sizeof(Vector3)),
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const tangentBufData{
    .pSysMem = mCpuData->tangents.data()
  };

  hr = gRenderer.GetDevice()->CreateBuffer(&tangentBufDesc, &tangentBufData, mTangentBuf.ReleaseAndGetAddressOf());
  assert(SUCCEEDED(hr));
}


auto Mesh::CalculateBounds() noexcept -> void {
  mBounds.min = Vector3{std::numeric_limits<float>::max()};
  mBounds.max = Vector3{std::numeric_limits<float>::lowest()};

  for (auto const& position : mCpuData->positions) {
    mBounds.min = Min(mBounds.min, position);
    mBounds.max = Max(mBounds.max, position);
  }
}


auto Mesh::EnsureCpuMemory() noexcept -> void {
  if (!mCpuData) {
    mCpuData = std::make_unique<GeometryData>();
  }
}


auto Mesh::Set16BitIndicesFrom32BitBuffer(std::span<std::uint32_t const> const indices) noexcept -> void {
  assert(HasCpuMemory());
  mCpuData->indices16.clear();
  std::ranges::transform(indices, std::back_inserter(mCpuData->indices16), [](std::uint32_t const idx) {
    return static_cast<std::uint16_t>(idx);
  });
  mCpuData->indices32.clear();
  mIdxFormat = DXGI_FORMAT_R16_UINT;
}


Mesh::Mesh(Data data, bool const keepDataInCpuMemory) noexcept {
  SetData(std::move(data));
  std::ignore = ValidateAndUpdate(keepDataInCpuMemory);
}


auto Mesh::GetPositions() const noexcept -> std::span<Vector3 const> {
  return mCpuData ? mCpuData->positions : std::span<Vector3 const>{};
}


auto Mesh::SetPositions(std::span<Vector3 const> positions) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->positions.assign(std::begin(positions), std::end(positions));
}


auto Mesh::SetPositions(std::vector<Vector3>&& positions) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->positions = std::move(positions);
}


auto Mesh::GetNormals() const noexcept -> std::span<Vector3 const> {
  return mCpuData ? mCpuData->normals : std::span<Vector3 const>{};
}


auto Mesh::SetNormals(std::span<Vector3 const> normals) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->normals.assign(std::begin(normals), std::end(normals));
}


auto Mesh::SetNormals(std::vector<Vector3>&& normals) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->normals = std::move(normals);
}


auto Mesh::GetUVs() const noexcept -> std::span<Vector2 const> {
  return mCpuData ? mCpuData->uvs : std::span<Vector2 const>{};
}


auto Mesh::SetUVs(std::span<Vector2 const> uvs) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->uvs.assign(std::begin(uvs), std::end(uvs));
}


auto Mesh::SetUVs(std::vector<Vector2>&& uvs) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->uvs = std::move(uvs);
}


auto Mesh::GetTangents() const noexcept -> std::span<Vector3 const> {
  return mCpuData ? mCpuData->tangents : std::span<Vector3 const>{};
}


auto Mesh::SetTangents(std::span<Vector3 const> tangents) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->tangents.assign(std::begin(tangents), std::end(tangents));
}


auto Mesh::SetTangents(std::vector<Vector3>&& tangents) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->tangents = std::move(tangents);
}


auto Mesh::GetIndices16() const noexcept -> std::span<std::uint16_t const> {
  return mCpuData ? mCpuData->indices16 : std::span<std::uint16_t const>{};
}


auto Mesh::GetIndices32() const noexcept -> std::span<std::uint32_t const> {
  return mCpuData ? mCpuData->indices32 : std::span<std::uint32_t const>{};
}


auto Mesh::SetIndices(std::span<std::uint16_t const> const indices) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->indices16.assign(std::begin(indices), std::end(indices));
  mCpuData->indices32.clear();
  mIdxFormat = DXGI_FORMAT_R16_UINT;
}


auto Mesh::SetIndices(std::span<std::uint32_t const> const indices) noexcept -> void {
  EnsureCpuMemory();

  for (auto const idx : indices) {
    if (idx > std::numeric_limits<std::uint16_t>::max()) {
      mCpuData->indices16.clear();
      mCpuData->indices32.assign(std::begin(indices), std::end(indices));
      mIdxFormat = DXGI_FORMAT_R32_UINT;
      return;
    }
  }

  Set16BitIndicesFrom32BitBuffer(indices);
}


auto Mesh::SetIndices(std::vector<std::uint16_t>&& indices) noexcept -> void {
  EnsureCpuMemory();
  mCpuData->indices16 = std::move(indices);
  mCpuData->indices32.clear();
  mIdxFormat = DXGI_FORMAT_R16_UINT;
}


auto Mesh::SetIndices(std::vector<std::uint32_t>&& indices) noexcept -> void {
  EnsureCpuMemory();

  for (auto const idx : indices) {
    if (idx > std::numeric_limits<std::uint16_t>::max()) {
      mCpuData->indices16.clear();
      mCpuData->indices32 = std::move(indices);
      mIdxFormat = DXGI_FORMAT_R32_UINT;
      return;
    }
  }

  Set16BitIndicesFrom32BitBuffer(indices);
}


auto Mesh::GetSubMeshes() const noexcept -> std::span<SubMeshData const> {
  return mSubmeshes;
}


auto Mesh::SetSubMeshes(std::span<SubMeshData const> submeshes) noexcept -> void {
  mSubmeshes.assign(std::begin(submeshes), std::end(submeshes));
}


auto Mesh::SetSubmeshes(std::vector<SubMeshData>&& submeshes) noexcept -> void {
  mSubmeshes = std::move(submeshes);
}


auto Mesh::GetBounds() const noexcept -> AABB const& {
  return mBounds;
}


auto Mesh::SetData(Data const& data) noexcept -> void {
  EnsureCpuMemory();
  SetPositions(data.positions);
  SetNormals(data.normals);
  SetUVs(data.uvs);
  SetTangents(data.tangents);
  SetIndices(data.indices);
  SetSubMeshes(data.subMeshes);
}


auto Mesh::SetData(Data&& data) noexcept -> void {
  EnsureCpuMemory();
  SetPositions(std::move(data.positions));
  SetNormals(std::move(data.normals));
  SetUVs(std::move(data.uvs));
  SetTangents(std::move(data.tangents));
  SetIndices(std::move(data.indices));
  SetSubMeshes(std::move(data.subMeshes));
}


auto Mesh::ValidateAndUpdate(bool const keepDataInCpuMemory) noexcept -> bool {
  if (!mCpuData) {
    return true;
  }

  if (mCpuData->positions.size() != mCpuData->normals.size() || mCpuData->normals.size() != mCpuData->uvs.size() || mCpuData->uvs.size() != mCpuData->tangents.size() ||
      mCpuData->positions.empty() || (mCpuData->indices16.empty() && mCpuData->indices32.empty())) {
    return false;
  }

  for (auto const& [baseVertex, firstIdx, idxCount, mtlSlotName] : mSubmeshes) {
    if (baseVertex >= std::ssize(mCpuData->positions)) {
      return false;
    }

    if (firstIdx >= std::ssize(mCpuData->indices16) && firstIdx >= std::ssize(mCpuData->indices32)) {
      return false;
    }

    auto const lastIdx{firstIdx + idxCount - 1};

    if (lastIdx >= std::ssize(mCpuData->indices16) && lastIdx >= std::ssize(mCpuData->indices32)) {
      return false;
    }

    for (auto i{firstIdx}; i <= lastIdx; i++) {
      if (auto const idx{std::ssize(mCpuData->indices16) > i ? mCpuData->indices16[i] : mCpuData->indices32[i]}; idx + baseVertex > std::size(mCpuData->positions)) {
        return false;
      }
    }
  }

  mVertexCount = static_cast<int>(std::ssize(mCpuData->positions));
  mIndexCount = static_cast<int>(mCpuData->indices16.empty() ? std::ssize(mCpuData->indices32) : std::ssize(mCpuData->indices16));
  mSubmeshCount = static_cast<int>(std::ssize(mSubmeshes));

  CalculateBounds();
  UploadToGpu();

  if (!keepDataInCpuMemory) {
    ReleaseCpuMemory();
  }

  return true;
}


auto Mesh::HasCpuMemory() const noexcept -> bool {
  return static_cast<bool>(mCpuData);
}


auto Mesh::ReleaseCpuMemory() noexcept -> void {
  mCpuData.reset();
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


auto Mesh::GetTangentBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
  return mTangentBuf;
}


auto Mesh::GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
  return mIdxBuf;
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


auto Mesh::GetIndexFormat() const noexcept -> DXGI_FORMAT {
  return mIdxFormat;
}


auto Mesh::OnDrawProperties(bool& changed) -> void {
  Resource::OnDrawProperties(changed);

  ImGui::Text("%s: %d", "Vertex Count", GetVertexCount());
  ImGui::Text("%s: %d", "Index Count", GetIndexCount());
  ImGui::Text("%s: %d", "Submesh Count", GetSubmeshCount());
}
}
