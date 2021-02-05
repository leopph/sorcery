#pragma once

#include "light.h"
#include "behavior.h"

namespace leopph
{
	// TODO
	class LEOPPHAPI PointLight final : private implementation::Light, public Behavior
	{
	public:
		PointLight();
		virtual ~PointLight() override;

		void operator()() {}


		float Range() const;
		void Range(float newRange);

	private:
		const float m_Constant{ 1.0f };
		float m_Linear;
		const float m_Quadratic{ 0.2f };
	};
}