#pragma once

#include "leopphapi.h"
#include "object.h"

namespace leopph
{
	// TODO IMPLEMENT THIS
	class LEOPPHAPI PointLight final : public Object
	{
	public:
		float Range() const;
		void Range(float newRange);

	private:
		const float m_Constant{ 1.0f };
		float m_Linear;
		const float m_Quadratic{ 0.2f };
	};
}