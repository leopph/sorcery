#pragma once

#include "Light.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../entity/Entity.hpp"


namespace leopph
{
	// DirectionalLights are special Lights that provide a way to light your scene with a source that appears to be infinitely far.
	// The Position of the DirectionalLight does not take part in calculations, it always affects objects from the direction it is set to.
	class DirectionalLight final : public Light
	{
		public:
			// The direction where the DirectionalLight shines.
			// This is exactly the same as the owning Entity's forward, or Z vector.
			[[nodiscard]] LEOPPHAPI
			auto Direction() const noexcept -> Vector3 const&;

			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The returned value is always non-negative.
			[[nodiscard]] LEOPPHAPI
			auto ShadowExtension() const noexcept -> float;

			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The input will be clamped to the range [0, inf).
			LEOPPHAPI
			auto ShadowExtension(float newRange) -> void;

			LEOPPHAPI
			auto Owner(Entity* entity) -> void override;
			using Light::Owner;

			LEOPPHAPI
			auto Active(bool active) -> void override;
			using Light::Active;

			DirectionalLight() = default;

			DirectionalLight(DirectionalLight const& other) = default;
			LEOPPHAPI
			auto operator=(DirectionalLight const& other) -> DirectionalLight&;

			DirectionalLight(DirectionalLight&& other) = delete;
			auto operator=(DirectionalLight&& other) -> void = delete;

			LEOPPHAPI ~DirectionalLight() override;

		private:
			float m_ShadowRange{50.f};
	};
}
