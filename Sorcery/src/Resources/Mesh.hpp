#pragma once

#include "Resource.hpp"
#include "../Math.hpp"
#include "../Bounds.hpp"

#include <span>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace sorcery {
class Mesh final : public Resource {
  RTTR_ENABLE(Resource)
  struct GeometryData {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector3> tangents;
    std::vector<u32> indices;
  };

public:
  struct SubMeshData {
    int baseVertex;
    int firstIndex;
    int indexCount;

    std::string mtlSlotName;
  };


  struct Data {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector3> tangents;
    std::vector<u32> indices;
    std::vector<SubMeshData> subMeshes;
  };

private:
  GeometryData* mCPUData{ new GeometryData{} };
  std::vector<SubMeshData> mSubmeshes;
  AABB mBounds{};
  int mVertexCount{ 0 };
  int mIndexCount{ 0 };
  int mSubmeshCount{ 0 };
  Microsoft::WRL::ComPtr<ID3D11Buffer> mPosBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mNormBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mUvBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mTangentBuf;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mIndBuf;

  auto UploadToGPU() -> void;
  auto CalculateBounds() -> void;

public:
  Mesh() = default;

  LEOPPHAPI explicit Mesh(Data data, bool keepDataInCPUMemory = false);

  [[nodiscard]] LEOPPHAPI auto GetPositions() const noexcept -> std::span<Vector3 const>;
  LEOPPHAPI auto SetPositions(std::vector<Vector3> positions, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetNormals() const noexcept -> std::span<Vector3 const>;
  LEOPPHAPI auto SetNormals(std::vector<Vector3> normals, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetUVs() const noexcept -> std::span<Vector2 const>;
  LEOPPHAPI auto SetUVs(std::vector<Vector2> uvs, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetTangents() const noexcept -> std::span<Vector3 const>;
  LEOPPHAPI auto SetTangents(std::vector<Vector3> tangents, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetIndices() const noexcept -> std::span<u32 const>;
  LEOPPHAPI auto SetIndices(std::vector<u32> indices, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSubMeshes() const noexcept -> std::span<SubMeshData const>;
  LEOPPHAPI auto SetSubMeshes(std::vector<SubMeshData> subMeshes, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetBounds() const noexcept -> AABB const&;

  LEOPPHAPI auto ValidateAndUpdate(bool keepDataInCPUMemory = false) -> void;

  [[nodiscard]] LEOPPHAPI auto GetPositionBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetNormalBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetUVBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
  [[nodiscard]] LEOPPHAPI auto GetTangentBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;

  LEOPPHAPI auto SetData(Data data, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  LEOPPHAPI auto ReleaseCPUMemory() -> void;
  [[nodiscard]] LEOPPHAPI auto HasCPUMemory() const noexcept -> bool;

  [[nodiscard]] LEOPPHAPI auto GetVertexCount() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetIndexCount() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetSubmeshCount() const noexcept -> int;

  LEOPPHAPI auto OnDrawProperties() -> void override;
};
}
