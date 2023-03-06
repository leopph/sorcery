#pragma once

#include "AABB.hpp"
#include "Color.hpp"
#include "Object.hpp"
#include "Util.hpp"
#include "Texture2D.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace leopph {
class Material final : public Object {
	struct BufferData {
		Vector3 albedo{ 1, 1, 1 };
		f32 metallic{ 0 };
		f32 roughness{ 0.5f };
		f32 ao{ 1 };
		int sampleAlbedo{ 0 };
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> mBuffer;
	BufferData mBufData;

	Texture2D* mAlbedoMap{ nullptr };

	auto UpdateGPUData() const noexcept -> void;

public:
	LEOPPHAPI Material();

	LEOPPHAPI [[nodiscard]] auto GetAlbedoVector() const noexcept -> Vector3;
	LEOPPHAPI auto SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetAlbedoColor() const noexcept -> Color;
	LEOPPHAPI auto SetAlbedoColor(Color const& albedoColor) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetMetallic() const noexcept -> f32;
	LEOPPHAPI auto SetMetallic(f32 metallic) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetRoughness() const noexcept -> f32;
	LEOPPHAPI auto SetRoughness(f32 roughness) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetAo() const noexcept -> f32;
	LEOPPHAPI auto SetAo(f32 ao) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetAlbedoMap() const noexcept -> Texture2D*;
	LEOPPHAPI auto SetAlbedoMap(Texture2D* tex) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*>;

	LEOPPHAPI auto BindPs() const noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Type const SerializationType;

	LEOPPHAPI auto SerializeBinary(std::vector<u8>& out) const -> void override;
	LEOPPHAPI auto DeserializeBinary(std::span<u8 const> bytes) -> BinaryDeserializationResult override;
};
}
