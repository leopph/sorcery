#pragma once

#include <filesystem>

#include "AABB.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Object.hpp"
#include "Renderer.hpp"
#include "Util.hpp"


namespace leopph {
	class Material final : public Object {
		struct BufferData {
			Vector3 albedo;
			f32 metallic;
			f32 roughness;
			f32 ao;
			i32 albedoMapIndex;
			i32 metallicMapIndex;
			i32 roughnessMapIndex;
			i32 aoMapIndex;
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> mBuffer;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mAlbedoMap;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mMetallicMap;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mRoughnessMap;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mAoMap;

	public:
		LEOPPHAPI Material();

		LEOPPHAPI auto SetAlbedoColor(Color const& albedoColor) const noexcept -> void;
		LEOPPHAPI auto SetAlbedoMap(ID3D11Texture2D* albedoMap) noexcept -> void;
		LEOPPHAPI auto SetMetallic(f32 metallic) const noexcept -> void;
		LEOPPHAPI auto SetMetallicMap(ID3D11Texture2D* metallicMap) noexcept -> void;
		LEOPPHAPI auto SetRoughness(f32 roughness) const noexcept -> void;
		LEOPPHAPI auto SetRoughnessMap(ID3D11Texture2D* roughnessMap) noexcept -> void;
		LEOPPHAPI auto SetAo(f32 ao) const noexcept -> void;
		LEOPPHAPI auto SetAoMap(ID3D11Texture2D* aoMap) noexcept -> void;

		LEOPPHAPI [[nodiscard]] auto GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*>;
		LEOPPHAPI [[nodiscard]] auto GetAlbedoMap() const noexcept -> NonOwning<ID3D11Texture2D*>;
		LEOPPHAPI [[nodiscard]] auto GetMetallicMap() const noexcept -> NonOwning<ID3D11Texture2D*>;
		LEOPPHAPI [[nodiscard]] auto GetRoughnessMap() const noexcept -> NonOwning<ID3D11Texture2D*>;
		LEOPPHAPI [[nodiscard]] auto GetAoMap() const noexcept -> NonOwning<ID3D11Texture2D*>;

		LEOPPHAPI auto BindPs() const noexcept -> void;
	};


	class Texture final : public Object {
	public:
		enum class Type {
			OneDimensional,
			TwoDimensional,
			ThreeDimensional
		};

		LEOPPHAPI explicit Texture(Image const& img);
	};


	class Mesh final : public Object {
		
	};


	class Model final : public Object {

	};
}