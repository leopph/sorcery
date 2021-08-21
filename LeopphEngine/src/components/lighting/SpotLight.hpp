#pragma once

#include "Light.hpp"

namespace leopph
{
	class Object;

	/* A SpotLight is a kind of light that shines in a cone.
	* It has a position, an orientation, and a radius. 
	* SpotLights always point in the direction of the owning Object's Forward vector. */
	class SpotLight final : public impl::Light
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