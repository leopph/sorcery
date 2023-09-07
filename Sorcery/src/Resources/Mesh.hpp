#pragma once

#include "Resource.hpp"
#include "../Math.hpp"
#include "../Bounds.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>


namespace sorcery {
class Mesh final : public Resource {
  RTTR_ENABLE(Resource)
  struct GeometryData {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector3> tangents;
    std::vector<std::uint16_t> indices16;
    std::vector<std::uint32_t> indices32;
  };

public:
  struct MaterialSlotInfo {
    std::string name;
  };


  struct SubMeshInfo {
    int baseVertex;
    int firstIndex;
    int indexCount;
    int materialIndex;
    AABB bounds;
  };


  struct Data {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector3> tangents;
    std::variant<std::vector<std::uint16_t>, std::vector<std::uint32_t>> indices;
    std::vector<MaterialSlotInfo> materialSlots;
    std::vector<SubMeshInfo> subMeshes;
  };

private:
  std::unique_ptr<GeometryData> mCpuData{nullptr};
  std::vector<SubMeshInfo> mSubmeshes;
  std::vector<MaterialSlotInfo> mMtlSlots;
  AABB mBounds{};
  Microsoft::WRL::ComPtr<ID3D11Buffer> mPosBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mNormBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mUvBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mTangentBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mIdxBuf;
  int mVertexCount{0};
  int mIndexCount{0};
  int mSubmeshCount{0};
  DXGI_FORMAT mIdxFormat{DXGI_FORMAT_R16_UINT};

  auto UploadToGpu() noexcept -> void;
  auto CalculateBounds() noexcept -> void;
  auto EnsureCpuMemory() noexcept -> void;
  auto Set16BitIndicesFrom32BitBuffer(std::span<std::uint32_t const> indices) noexcept -> void;

public:
  Mesh() = default;
  LEOPPHAPI explicit Mesh(Data data, bool keepDataInCpuMemory = false) noexcept;

  [[nodiscard]] LEOPPHAPI auto GetPositions() const noexcept -> std::span<Vector3 const>;
  LEOPPHAPI auto SetPositions(std::span<Vector3 const> positions) noexcept -> void;
  LEOPPHAPI auto SetPositions(std::vector<Vector3>&& positions) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetNormals() const noexcept -> std::span<Vector3 const>;
  LEOPPHAPI auto SetNormals(std::span<Vector3 const> normals) noexcept -> void;
  LEOPPHAPI auto SetNormals(std::vector<Vector3>&& normals) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetUVs() const noexcept -> std::span<Vector2 const>;
  LEOPPHAPI auto SetUVs(std::span<Vector2 const> uvs) noexcept -> void;
  LEOPPHAPI auto SetUVs(std::vector<Vector2>&& uvs) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetTangents() const noexcept -> std::span<Vector3 const>;
  LEOPPHAPI auto SetTangents(std::span<Vector3 const> tangents) noexcept -> void;
  LEOPPHAPI auto SetTangents(std::vector<Vector3>&& tangents) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetIndices16() const noexcept -> std::span<std::uint16_t const>;
  [[nodiscard]] LEOPPHAPI auto GetIndices32() const noexcept -> std::span<std::uint32_t const>;
  LEOPPHAPI auto SetIndices(std::span<std::uint16_t const> indices) noexcept -> void;
  LEOPPHAPI auto SetIndices(std::span<std::uint32_t const> indices) noexcept -> void;
  LEOPPHAPI auto SetIndices(std::vector<std::uint16_t>&& indices) noexcept -> void;
  LEOPPHAPI auto SetIndices(std::vector<std::uint32_t>&& indices) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetMaterialSlots() const noexcept -> std::span<MaterialSlotInfo const>;
  LEOPPHAPI auto SetMaterialSlots(std::span<MaterialSlotInfo const> mtlSlots) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSubMeshes() const noexcept -> std::span<SubMeshInfo const>;
  LEOPPHAPI auto SetSubMeshes(std::span<SubMeshInfo const> submeshes) noexcept -> void;
  LEOPPHAPI auto SetSubmeshes(std::vector<SubMeshInfo>&& submeshes) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetBounds() const noexcept -> AABB const&;

  LEOPPHAPI auto SetData(Data const& data) noexcept -> void;
  LEOPPHAPI auto SetData(Data&& data) noexcept -> void;
  [[nodiscard]] LEOPPHAPI auto ValidateAndUpdate(bool keepDataInCpuMemory = false) noexcept -> bool;

  [[nodiscard]] LEOPPHAPI auto HasCpuMemory() const noexcept -> bool;
  LEOPPHAPI auto ReleaseCpuMemory() noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetPositionBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetNormalBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetUVBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetTangentBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;

  [[nodiscard]] LEOPPHAPI auto GetVertexCount() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetIndexCount() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetSubmeshCount() const noexcept -> int;

  [[nodiscard]] LEOPPHAPI auto GetIndexFormat() const noexcept -> DXGI_FORMAT;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
