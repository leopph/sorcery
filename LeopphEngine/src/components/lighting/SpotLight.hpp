#pragma once

#include "AttenuatedLight.hpp"

namespace leopph
{
	class Entity;

	/* A SpotLight is a kind of light that shines in a cone.
	* It has a position, attenuation, an orientation, and a radius. 
	* SpotLights always point in the direction of the owning Entity's Transform's Forward vector.
	* See "AttenuatedLight.hpp", "Light.hpp", and "Component.hpp" for more information.
	*/
	class SpotLight final : public impl::AttenuatedLight
	{
	public:
		LEOPPHAPI explicit SpotLight(Entity& owner);
		LEOPPHAPI ~SpotLight() override;

		SpotLight(const SpotLight&) = delete;
		SpotLight(SpotLight&&) = delete;
		void operator=(const SpotLight&) = delete;
		void operator=(SpotLight&&) = delete;

		/* The angle in degrees at which the light starts to fade out */
		[[nodiscard]] LEOPPHAPI float InnerAngle() const;
		LEOPPHAPI void InnerAngle(float degrees);

		/* The angle in degrees at which the light is completely cut */
		[[nodiscard]] LEOPPHAPI float OuterAngle() const;
		LEOPPHAPI void OuterAngle(float degrees);

	private:
		float m_InnerAngle;
		float m_OuterAngle;
	};
}