#pragma once

#include "light.h"
#include "behavior.h"
#include "vector.h"

namespace leopph
{
#pragma warning(push)
#pragma warning (disable: 4275)
#pragma warning (disable: 4251)

	// POINT LIGHT SOURCE
	class LEOPPHAPI PointLight final : public implementation::Light
	{
	public:
		PointLight(Object& object);

		void operator()() {}


		float Range() const;
		
		float Constant() const;
		float Linear() const;
		float Quadratic() const;

		void Constant(float value);
		void Linear(float value);
		void Quadratic(float value);

	private:
		float m_Constant{ 1.0f };
		float m_Linear{ 0.22f };
		float m_Quadratic{ 0.2f };
	};

#pragma warning(pop)
}