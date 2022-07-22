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
			[[nodiscard]] LEOPPHAPI Vector3 const& Direction() const noexcept;


			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The returned value is always non-negative.
			[[nodiscard]] LEOPPHAPI f32 ShadowExtension() const noexcept;

			// This value is used as an offset on the shadow cascades' bounding boxes
			// to extend shadowing to occluders not visible to the active Camera.
			// The input will be clamped to the range [0, inf).
			LEOPPHAPI void ShadowExtension(f32 newRange);


			LEOPPHAPI void Owner(Entity* entity) override;
			using Light::Owner;


			LEOPPHAPI void Active(bool active) override;
			using Light::Active;


			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

			DirectionalLight() = default;

			DirectionalLight(DirectionalLight const& other) = default;
			LEOPPHAPI DirectionalLight& operator=(DirectionalLight const& other);

			DirectionalLight(DirectionalLight&& other) = delete;
			void operator=(DirectionalLight&& other) = delete;

			LEOPPHAPI ~DirectionalLight() override;

		private:
			f32 m_ShadowRange{50.f};
	};
}
