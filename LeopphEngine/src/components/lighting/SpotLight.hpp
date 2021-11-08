#pragma once

#include "AttenuatedLight.hpp"

namespace leopph
{
	class Entity;

	/* SpotLights are special Lights that shine in a cone.
	 * They have position, orientation, attenuation, and radius. */
	class SpotLight final : public impl::AttenuatedLight
	{
	public:
		// Get the angle in degrees at which the light starts to fade out.
		[[nodiscard]]
		LEOPPHAPI float InnerAngle() const;

		// Set the angle in degrees at which the light starts to fade out.
		LEOPPHAPI void InnerAngle(float degrees);

		// Get the angle in degrees at which the light is completely cut.
		[[nodiscard]]
		LEOPPHAPI float OuterAngle() const;

		// Set the angle in degrees at which the light is completely cut.
		LEOPPHAPI void OuterAngle(float degrees);


		LEOPPHAPI explicit SpotLight(leopph::Entity& owner);
		SpotLight(const SpotLight&) = delete;
		SpotLight(SpotLight&&) = delete;

		LEOPPHAPI ~SpotLight() override;

		void operator=(const SpotLight&) = delete;
		void operator=(SpotLight&&) = delete;


	private:
		float m_InnerAngle;
		float m_OuterAngle;
	};
}