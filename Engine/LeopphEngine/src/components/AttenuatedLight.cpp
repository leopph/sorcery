#include "AttenuatedLight.hpp"


namespace leopph
{
	auto AttenuatedLight::Range() const noexcept -> f32
	{
		return m_Range;
	}


	auto AttenuatedLight::Range(f32 const value) noexcept -> void
	{
		m_Range = value;
	}
}
