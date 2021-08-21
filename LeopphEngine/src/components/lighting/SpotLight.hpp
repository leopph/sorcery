#pragma once

#include "AttenuatedLight.hpp"

namespace leopph
{
	class Object;

	/* A SpotLight is a kind of light that shines in a cone.
	* It has a position, attenuation, an orientation, and a radius. 
	* SpotLights always point in the direction of the owning Object's Forward vector.
	* See "AttenuatedLight.hpp", "Light.hpp", and "Component.hpp" for more information.
	*/
	class SpotLight final : public impl::AttenuatedLight
	{
	public:
		LEOPPHAPI explicit SpotLight(Object& owner);
		LEOPPHAPI ~SpotLight() override;

		SpotLight(const SpotLight&) = delete;
		SpotLight(SpotLight&&) = delete;
		void operator=(const SpotLight&) = delete;
		void operator=(SpotLight&&) = delete;

		/* The cutoff angle in degrees */
		[[nodiscard]] LEOPPHAPI float Angle() const;
		LEOPPHAPI void Angle(float degrees);

	private:
		float m_Angle;
	};
}