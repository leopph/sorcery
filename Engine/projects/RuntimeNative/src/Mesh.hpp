#pragma once

#include "Object.hpp"
#include "Renderer.hpp"
#include "Math.hpp"

#include <span>
#include <vector>


namespace leopph {
	class Mesh final : public Object {
	public:
		struct Data {
			std::vector<Vector3> positions;
			std::vector<Vector3> normals;
			std::vector<Vector2> uvs;
			std::vector<u32> indices;
		};

	private:
		Data mData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPosBuf;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mNormBuf;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mUvBuf;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndBuf;

		auto UploadToGPU() -> void;

	public:
		LEOPPHAPI explicit Mesh(Data data);

		LEOPPHAPI [[nodiscard]] auto GetPositions() const noexcept -> std::span<Vector3 const>;
		LEOPPHAPI auto SetPositions(std::vector<Vector3> positions) -> void;

		LEOPPHAPI [[nodiscard]] auto GetNormals() const noexcept -> std::span<Vector3 const>;
		LEOPPHAPI auto SetNormals() const noexcept -> std::span<Vector3 const>;

		[[nodiscard]] auto GetSerializationType() const -> Type override;

		auto SerializeBinary(std::vector<u8>& out) const -> void override;
		[[nodiscard]] auto DeserializeBinary(std::span<u8 const> bytes) -> BinaryDeserializationResult override;
	};
}