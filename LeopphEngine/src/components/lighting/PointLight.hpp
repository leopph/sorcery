#pragma once

#include "Light.hpp"

namespace leopph
{
	class Object;

	/*------------------------------------------------------------------------------------
	Point lights provide a way to light your scene in a way that spreads in all directions.
	Positioning matters, and the further the light has to travel, the weaker it gets.
	See "component.h" for more information.
	------------------------------------------------------------------------------------*/
	class PointLight final : public impl::Light
	{
	public:
		/* Internally used functions */
		LEOPPHAPI explicit PointLight(Object& owner);
		LEOPPHAPI ~PointLight() override;

		PointLight(const PointLight&) = delete;
		PointLight(PointLight&&) = delete;
		void operator=(const PointLight&) = delete;
		void operator=(PointLight&&) = delete;
		
		/* Point lights are defined by 3 terms used to calculate its strength.
		You can try different combinations until you find one that fits your needs. */
		LEOPPHAPI float Constant() const;
		LEOPPHAPI float Linear() const;
		LEOPPHAPI float Quadratic() const;

		LEOPPHAPI void Constant(float value);
		LEOPPHAPI void Linear(float value);
		LEOPPHAPI void Quadratic(float value);

	private:
		float m_Constant;
		float m_Linear;
		float m_Quadratic;
	};
}