#pragma once

#include "light.h"

namespace leopph
{
	// POINT LIGHT SOURCE
	class  PointLight final : public impl::Light
	{
	public:
		LEOPPHAPI PointLight(leopph::Object& object);

		LEOPPHAPI float Range() const;
		
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