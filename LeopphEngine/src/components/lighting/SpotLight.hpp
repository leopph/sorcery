#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	/* SpotLights are special Lights that shine in a cone.
	 * They have Position, orientation, attenuation, and radius. */
	class SpotLight final : public internal::AttenuatedLight
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


			LEOPPHAPI explicit SpotLight(leopph::Entity* entity);

			SpotLight(const SpotLight&) = delete;
			void operator=(const SpotLight&) = delete;

			SpotLight(SpotLight&&) = delete;
			void operator=(SpotLight&&) = delete;

			LEOPPHAPI ~SpotLight() override;


		private:
			float m_InnerAngle;
			float m_OuterAngle;
	};
}
