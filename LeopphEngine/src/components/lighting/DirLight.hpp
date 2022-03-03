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
			[[nodiscard]] constexpr auto Direction() const noexcept -> auto&;

			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The returned value is always non-negative.
			[[nodiscard]] constexpr auto ShadowExtension() const noexcept;

			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The input will be clamped to the range [0, inf).
			LEOPPHAPI auto ShadowExtension(float newRange) -> void;

			LEOPPHAPI auto Activate() -> void override;
			LEOPPHAPI auto Deactivate() -> void override;

			LEOPPHAPI explicit DirectionalLight(leopph::Entity* entity);

			DirectionalLight(const DirectionalLight&) = delete;
			auto operator=(const DirectionalLight&) -> void = delete;

			DirectionalLight(DirectionalLight&&) = delete;
			auto operator=(DirectionalLight&&) -> void = delete;

			LEOPPHAPI ~DirectionalLight() override;

		private:
			float m_ShadowRange{50.f};
	};


	constexpr auto DirectionalLight::Direction() const noexcept -> auto&
	{
		return Entity()->Transform()->Forward();
	}


	constexpr auto DirectionalLight::ShadowExtension() const noexcept
	{
		return m_ShadowRange;
	}
}
