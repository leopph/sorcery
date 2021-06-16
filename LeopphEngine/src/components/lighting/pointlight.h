#pragma once

#include "light.h"

namespace leopph
{
	/*------------------------------------------------------------------------------------
	Pointlights provide a way to light your scene in a way that spreads in all directions.
	Positoning matters, and the further the light has to travel, the weaker it gets.
	See "component.h" for more information.
	------------------------------------------------------------------------------------*/

	class PointLight final : public impl::Light
	{
	public:
		/* Internally used functions */
		LEOPPHAPI PointLight();
		LEOPPHAPI ~PointLight() override;

		/* The range at which the light completely diminishes */
		LEOPPHAPI float Range() const;
		
		/* Pointslights are defined by 3 terms used to calculate its strength.
		You can try different combinations until you find one that fits your needs. */
		LEOPPHAPI float Constant() const;
		LEOPPHAPI float Linear() const;
		LEOPPHAPI float Quadratic() const;

		LEOPPHAPI void Constant(float value);
		LEOPPHAPI void Linear(float value);
		LEOPPHAPI void Quadratic(float value);

	private:
		float m_Constant{ 1.0f };
		float m_Linear{ 0.22f };
		float m_Quadratic{ 0.2f };
	};
}