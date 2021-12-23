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
			LEOPPHAPI float Constant() const;

			// Set the attenuation's constanst value.
			LEOPPHAPI void Constant(float value);

			/* Get the attenuation's linear value.
			 * This scales linearly with distance. */
			[[nodiscard]]
			LEOPPHAPI float Linear() const;

			/* Set the attenuation's linear value.
			 * This scales linearly with distance. */
			LEOPPHAPI void Linear(float value);

			/* Get the attenuation's quadratic value.
			 * This scales quadratically with distance. */
			[[nodiscard]]
			LEOPPHAPI float Quadratic() const;

			/* Set the attenuation's quadratic value.
			 * This scales quadratically with distance. */
			LEOPPHAPI void Quadratic(float value);


			LEOPPHAPI explicit AttenuatedLight(leopph::Entity* entity, float constant = 1.0f, float linear = 0.14f, float quadratic = 0.07f, float range = 32);

			AttenuatedLight(const AttenuatedLight&) = delete;
			void operator=(const AttenuatedLight&) = delete;

			AttenuatedLight(AttenuatedLight&&) = delete;
			void operator=(AttenuatedLight&&) = delete;

			LEOPPHAPI ~AttenuatedLight() override = 0;


		private:
			float m_Constant;
			float m_Linear;
			float m_Quadratic;
	};
}
