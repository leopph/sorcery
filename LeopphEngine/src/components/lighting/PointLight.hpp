#pragma once

#include "AttenuatedLight.hpp"

namespace leopph
{
	class Entity;

	/* A PointLight spreads in all directions, filling the scene.
	 * It has a position and attenuation.
	 * See "AttenuatedLight.hpp", "Light.hpp", and "Component.hpp" for more information
	 */
	class PointLight final : public impl::AttenuatedLight
	{
	public:
		LEOPPHAPI explicit PointLight(Entity& owner);
		LEOPPHAPI ~PointLight() override;

		PointLight(const PointLight&) = delete;
		PointLight(PointLight&&) = delete;
		void operator=(const PointLight&) = delete;
		void operator=(PointLight&&) = delete;
	};
}