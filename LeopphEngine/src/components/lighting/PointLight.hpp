#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	// PointLights are special Lights the effect of which spreads in all directions, filling the scene.
	// They have position and attenuation.
	class PointLight final : public AttenuatedLight
	{
		public:
			LEOPPHAPI
			auto Owner(Entity* entity) -> void override;
			using AttenuatedLight::Owner;

			LEOPPHAPI
			auto Active(bool active) -> void override;
			using AttenuatedLight::Active;

			PointLight() = default;

			PointLight(PointLight const& other) = default;

			LEOPPHAPI
			auto operator=(PointLight const& other) -> PointLight&;

			[[nodiscard]] LEOPPHAPI
			auto Clone() const -> ComponentPtr<> override;

			PointLight(PointLight&& other) = delete;
			auto operator=(PointLight&& other) -> void = delete;

			LEOPPHAPI ~PointLight() override;
	};
}
