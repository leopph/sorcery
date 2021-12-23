#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	/* PointLights are special Lights the effect of which spreads in all directions, filling the scene.
	 * They have Position and attenuation. */
	class PointLight final : public internal::AttenuatedLight
	{
		public:
			LEOPPHAPI explicit PointLight(leopph::Entity* entity);

			PointLight(const PointLight&) = delete;
			auto operator=(const PointLight&) -> void = delete;

			PointLight(PointLight&&) = delete;
			auto operator=(PointLight&&) -> void = delete;

			LEOPPHAPI ~PointLight() override;
	};
}
