#include "AttenuatedLight.hpp"


namespace leopph
{
	f32 AttenuatedLight::Range() const noexcept
	{
		return m_Range;
	}


	void AttenuatedLight::Range(f32 const value) noexcept
	{
		m_Range = value;
	}
}
