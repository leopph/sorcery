#pragma once

#include "Light.hpp"


namespace leopph::internal
{
	// Attenuated lights are special Lights whose intensity decreases the further photons travel.
	class AttenuatedLight : public Light
	{
		public:
			// Get the attenuations constanst value.
			[[nodiscard]]
			LEOPPHAPI auto Constant() const -> float;

			// Set the attenuation's constanst value.
			LEOPPHAPI auto Constant(float value) -> void;

			/* Get the attenuation's linear value.
			 * This scales linearly with distance. */
			[[nodiscard]]
			LEOPPHAPI auto Linear() const -> float;

			/* Set the attenuation's linear value.
			 * This scales linearly with distance. */
			LEOPPHAPI auto Linear(float value) -> void;

			/* Get the attenuation's quadratic value.
			 * This scales quadratically with distance. */
			[[nodiscard]]
			LEOPPHAPI auto Quadratic() const -> float;

			/* Set the attenuation's quadratic value.
			 * This scales quadratically with distance. */
			LEOPPHAPI auto Quadratic(float value) -> void;

			LEOPPHAPI explicit AttenuatedLight(leopph::Entity* entity, float constant = 1.0f, float linear = 0.14f, float quadratic = 0.07f, float range = 32);

			AttenuatedLight(const AttenuatedLight&) = delete;
			auto operator=(const AttenuatedLight&) -> void = delete;

			AttenuatedLight(AttenuatedLight&&) = delete;
			auto operator=(AttenuatedLight&&) -> void = delete;

			LEOPPHAPI ~AttenuatedLight() override = 0;

		private:
			float m_Constant;
			float m_Linear;
			float m_Quadratic;
	};
}
