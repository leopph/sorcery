#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	// PointLights are special Lights the effect of which spreads in all directions, filling the scene.
	// They have position and attenuation.
	class PointLight final : public AttenuatedLight
	{
		public:
			LEOPPHAPI PointLight();

			LEOPPHAPI auto Activate() -> void override;
			LEOPPHAPI auto Deactivate() -> void override;

			PointLight(const PointLight&) = delete;
			auto operator=(const PointLight&) -> void = delete;

			PointLight(PointLight&&) = delete;
			auto operator=(PointLight&&) -> void = delete;

			LEOPPHAPI ~PointLight() override;
	};
}
