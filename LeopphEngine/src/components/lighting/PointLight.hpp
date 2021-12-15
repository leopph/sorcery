#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	/* PointLights are special Lights the effect of which spreads in all directions, filling the scene.
	 * They have Position and attenuation. */
	class PointLight final : public impl::AttenuatedLight
	{
		public:
			LEOPPHAPI explicit PointLight(leopph::Entity* entity);

			PointLight(const PointLight&) = delete;
			void operator=(const PointLight&) = delete;

			PointLight(PointLight&&) = delete;
			void operator=(PointLight&&) = delete;

			LEOPPHAPI ~PointLight() override;
	};
}
