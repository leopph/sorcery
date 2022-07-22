#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	// PointLights are special Lights the effect of which spreads in all directions, filling the scene.
	// They have position and attenuation.
	class PointLight final : public AttenuatedLight
	{
		public:
			LEOPPHAPI void Owner(Entity* entity) override;
			using AttenuatedLight::Owner;

			LEOPPHAPI void Active(bool active) override;
			using AttenuatedLight::Active;

			PointLight() = default;

			PointLight(PointLight const& other) = default;

			LEOPPHAPI PointLight& operator=(PointLight const& other);

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

			PointLight(PointLight&& other) = delete;
			void operator=(PointLight&& other) = delete;

			LEOPPHAPI ~PointLight() override;
	};
}
