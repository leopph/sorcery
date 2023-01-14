#pragma once

#include "Resource.hpp"
#include "Math.hpp"

#include <span>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace leopph {
	class Mesh final : public Resource {
	public:
		struct Data {
			std::vector<Vector3> positions;
			std::vector<Vector3> normals;
			std::vector<Vector2> uvs;
			std::vector<u32> indices;
		};

	private:
		Data mData;
		Data mTempData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPosBuf;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mNormBuf;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mUvBuf;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndBuf;

		auto UploadToGPU() -> void;

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

		LEOPPHAPI auto ValidateAndUpdate() -> void;

		LEOPPHAPI [[nodiscard]] auto GetSerializationType() const -> Type override;

		LEOPPHAPI auto SerializeBinary(std::vector<u8>& out) const -> void override;
		LEOPPHAPI [[nodiscard]] auto DeserializeBinary(std::span<u8 const> bytes) -> BinaryDeserializationResult override;

		LEOPPHAPI [[nodiscard]] auto GetPositionBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
		LEOPPHAPI [[nodiscard]] auto GetNormalBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
		LEOPPHAPI [[nodiscard]] auto GetUVBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;
		LEOPPHAPI [[nodiscard]] auto GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer>;

		LEOPPHAPI Type constexpr static SerializationType{ Type::Mesh };
	};
}