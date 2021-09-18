#pragma once

#include "Light.hpp"



namespace leopph
{
	class Entity;



	namespace impl
	{
		/* Attenuated lights are special in the sense that their intensity
		 * is reduced over the distance. */
		class AttenuatedLight : public Light
		{
			public:
				LEOPPHAPI explicit AttenuatedLight(Entity& owner, float constant = 1.0f, float linear = 0.14f, float quadratic = 0.07f, float range = 32);
				LEOPPHAPI ~AttenuatedLight() override = 0;

				AttenuatedLight(const AttenuatedLight&) = delete;
				AttenuatedLight(AttenuatedLight&&) = delete;
				void operator=(const AttenuatedLight&) = delete;
				void operator=(AttenuatedLight&&) = delete;

				LEOPPHAPI float Constant() const;
				LEOPPHAPI void Constant(float value);

				LEOPPHAPI float Linear() const;
				LEOPPHAPI void Linear(float value);

				LEOPPHAPI float Quadratic() const;
				LEOPPHAPI void Quadratic(float value);


			private:
				float m_Constant;
				float m_Linear;
				float m_Quadratic;
		};
	}
}
