#pragma once

#include <filesystem>
#include <variant>

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
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> mBuffer;

	public:
		LEOPPHAPI Material();

		LEOPPHAPI auto SetAlbedoColor(Color const& albedoColor) const noexcept -> void;
		LEOPPHAPI auto SetMetallic(f32 metallic) const noexcept -> void;
		LEOPPHAPI auto SetRoughness(f32 roughness) const noexcept -> void;
		LEOPPHAPI auto SetAo(f32 ao) const noexcept -> void;

		LEOPPHAPI [[nodiscard]] auto GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*>;

		LEOPPHAPI auto BindPs() const noexcept -> void;
	};


	class Mesh final : public Object { };


	class Model final : public Object { };
}
