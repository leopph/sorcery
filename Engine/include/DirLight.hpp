#pragma once

#include "LeopphApi.hpp"
#include "Light.hpp"
#include "Types.hpp"


namespace leopph
{
	// DirectionalLights are special Lights that provide a way to light your scene with a source that appears to be infinitely far.
	// The Position of the DirectionalLight does not take part in calculations, it always affects objects from the direction it is set to.
	class DirectionalLight final : public Light
	{
		public:
			// The direction where the DirectionalLight shines.
			// This is exactly the same as the owning Entity's forward, or Z vector.
			[[nodiscard]] auto LEOPPHAPI Direction() const noexcept -> Vector3 const&;


			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The returned value is always non-negative.
			[[nodiscard]] auto LEOPPHAPI ShadowExtension() const noexcept -> f32;

			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The input will be clamped to the range [0, inf).
			auto LEOPPHAPI ShadowExtension(f32 newRange) -> void;

			
			auto LEOPPHAPI Owner(Entity* entity) -> void override;
			using Light::Owner;


			auto LEOPPHAPI Active(bool active) -> void override;
			using Light::Active;


			[[nodiscard]] auto LEOPPHAPI Clone() const -> ComponentPtr<> override;

			DirectionalLight() = default;

			DirectionalLight(DirectionalLight const& other) = default;
			auto LEOPPHAPI operator=(DirectionalLight const& other) -> DirectionalLight&;

			DirectionalLight(DirectionalLight&& other) = delete;
			auto operator=(DirectionalLight&& other) -> void = delete;

			LEOPPHAPI ~DirectionalLight() override;

		private:
			f32 m_ShadowRange{50.f};
	};
}
