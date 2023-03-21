#pragma once

#include "Object.hpp"
#include "Math.hpp"
#include "Bounds.hpp"

#include <span>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace leopph {
class Mesh final : public Object {
public:
	struct SubMeshData {
		int baseVertex;
		int firstIndex;
		int indexCount;
	};


	struct Data {
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> uvs;
		std::vector<u32> indices;
		std::vector<SubMeshData> subMeshes;
	};

private:
	Data mData;
	Data mTempData;
	AABB mBounds{};
	Microsoft::WRL::ComPtr<ID3D11Buffer> mPosBuf;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mNormBuf;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mUvBuf;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIndBuf;

	auto UploadToGPU() -> void;
	auto CalculateBounds() -> void;

public:
	Mesh() = default;

	LEOPPHAPI explicit Mesh(Data data);

	LEOPPHAPI [[nodiscard]] auto GetPositions() const noexcept -> std::span<Vector3 const>;
	LEOPPHAPI auto SetPositions(std::vector<Vector3> positions) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetNormals() const noexcept -> std::span<Vector3 const>;
	LEOPPHAPI auto SetNormals(std::vector<Vector3> normals) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetUVs() const noexcept -> std::span<Vector2 const>;
	LEOPPHAPI auto SetUVs(std::vector<Vector2> uvs) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetIndices() const noexcept -> std::span<u32 const>;
	LEOPPHAPI auto SetIndices(std::vector<u32> indices) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetSubMeshes() const noexcept -> std::span<SubMeshData const>;
	LEOPPHAPI auto SetSubMeshes(std::vector<SubMeshData> subMeshes) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetBounds() const noexcept -> AABB const&;

	LEOPPHAPI auto ValidateAndUpdate() -> void;

	LEOPPHAPI [[nodiscard]] auto GetSerializationType() const -> Type override;

	LEOPPHAPI [[nodiscard]] auto GetPositionBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
	LEOPPHAPI [[nodiscard]] auto GetNormalBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
	LEOPPHAPI [[nodiscard]] auto GetUVBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
	LEOPPHAPI [[nodiscard]] auto GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;

	LEOPPHAPI Type constexpr static SerializationType{ Type::Mesh };

	LEOPPHAPI void SetData(Data data) noexcept;
};
}
