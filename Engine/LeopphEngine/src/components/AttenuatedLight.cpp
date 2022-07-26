#include "AttenuatedLight.hpp"


namespace leopph
{
	f32 AttenuatedLight::get_range() const
	{
		return mRange;
	}



	void AttenuatedLight::set_range(f32 const value)
	{
		mRange = value;
	}
}
