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
			LEOPPHAPI auto InnerAngle() const -> float;

			// Set the angle in degrees at which the light starts to fade out.
			LEOPPHAPI auto InnerAngle(float degrees) -> void;

			// Get the angle in degrees at which the light is completely cut.
			[[nodiscard]]
			LEOPPHAPI auto OuterAngle() const -> float;

			// Set the angle in degrees at which the light is completely cut.
			LEOPPHAPI auto OuterAngle(float degrees) -> void;

			LEOPPHAPI explicit SpotLight(leopph::Entity* entity);

			SpotLight(const SpotLight&) = delete;
			auto operator=(const SpotLight&) -> void = delete;

			SpotLight(SpotLight&&) = delete;
			auto operator=(SpotLight&&) -> void = delete;

			LEOPPHAPI ~SpotLight() override;

		private:
			float m_InnerAngle;
			float m_OuterAngle;
	};
}
