#pragma once

#include "AABB.hpp"
#include "Color.hpp"
#include "NativeAsset.hpp"
#include "Util.hpp"
#include "Texture2D.hpp"

#include "shaders/ShaderInterop.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace leopph {
class Material final : public NativeAsset {
	ShaderMaterial mShaderMtl{
		.albedo = Vector3{ 1, 1, 1 },
		.metallic = 0.0f,
		.roughness = 0.5f,
		.ao = 1.0f,
		.sampleAlbedo = FALSE,
		.sampleMetallic = FALSE,
		.sampleRoughness = FALSE,
		.sampleAo = FALSE
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> mCB;

	Texture2D* mAlbedoMap{ nullptr };
	Texture2D* mMetallicMap{ nullptr };
	Texture2D* mRoughnessMap{ nullptr };
	Texture2D* mAoMap{ nullptr };

	auto UpdateGPUData() const noexcept -> void;
	auto CreateCB() -> void;

public:
	LEOPPHAPI Material();
	LEOPPHAPI Material(Vector3 const& albedoVector, float metallic, float roughness, float ao, Texture2D* albedoMap, Texture2D* metallicMap, Texture2D* roughnessMap, Texture2D* aoMap);

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

	LEOPPHAPI [[nodiscard]] auto GetMetallicMap() const noexcept -> Texture2D*;
	LEOPPHAPI auto SetMetallicMap(Texture2D* tex) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetRoughnessMap() const noexcept -> Texture2D*;
	LEOPPHAPI auto SetRoughnessMap(Texture2D* tex) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetAoMap() const noexcept -> Texture2D*;
	LEOPPHAPI auto SetAoMap(Texture2D* tex) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*>;

	LEOPPHAPI [[nodiscard]] auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Type const SerializationType;

	LEOPPHAPI auto Serialize(std::vector<std::uint8_t>& out) const noexcept -> void override;
	LEOPPHAPI auto Deserialize(std::span<std::uint8_t const> bytes) -> void override;
};
}
