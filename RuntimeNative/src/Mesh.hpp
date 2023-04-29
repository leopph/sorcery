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
		std::vector<Vector3> tangents;
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
	Microsoft::WRL::ComPtr<ID3D11Buffer> mTangentBuf;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIndBuf;

	auto UploadToGPU() -> void;
	auto CalculateBounds() -> void;

public:
	Mesh() = default;

	LEOPPHAPI explicit Mesh(Data data);

	[[nodiscard]] LEOPPHAPI auto GetPositions() const noexcept -> std::span<Vector3 const>;
	LEOPPHAPI auto SetPositions(std::vector<Vector3> positions) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetNormals() const noexcept -> std::span<Vector3 const>;
	LEOPPHAPI auto SetNormals(std::vector<Vector3> normals) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetUVs() const noexcept -> std::span<Vector2 const>;
	LEOPPHAPI auto SetUVs(std::vector<Vector2> uvs) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetTangents() const noexcept -> std::span<Vector3 const>;
	LEOPPHAPI auto SetTangents(std::vector<Vector3> tangents) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetIndices() const noexcept -> std::span<u32 const>;
	LEOPPHAPI auto SetIndices(std::vector<u32> indices) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetSubMeshes() const noexcept -> std::span<SubMeshData const>;
	LEOPPHAPI auto SetSubMeshes(std::vector<SubMeshData> subMeshes) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetBounds() const noexcept -> AABB const&;

	LEOPPHAPI auto ValidateAndUpdate() -> void;

	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

	[[nodiscard]] LEOPPHAPI auto GetPositionBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
	[[nodiscard]] LEOPPHAPI auto GetNormalBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
	[[nodiscard]] LEOPPHAPI auto GetUVBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
	[[nodiscard]] LEOPPHAPI auto GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
	[[nodiscard]] LEOPPHAPI auto GetTangentBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;

	LEOPPHAPI Type constexpr static SerializationType{ Type::Mesh };

	LEOPPHAPI auto SetData(Data data) noexcept -> void;
};
}
