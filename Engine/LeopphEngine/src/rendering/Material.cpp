#include "Material.hpp"

#include "Math.hpp"


namespace leopph
{
	bool Material::is_transparent() const
	{
		return mIsTransparent;
	}



	void Material::set_transparent(bool const transparent)
	{
		mIsTransparent = transparent;
	}



	bool BuiltInMaterial::is_two_sided() const
	{
		return mIsTwoSided;
	}



	void BuiltInMaterial::set_two_sided(bool const twoSided)
	{
		mIsTwoSided = twoSided;
	}



	f32 BuiltInMaterial::get_alpha_treshold() const
	{
		return mAlphaThreshold;
	}



	void BuiltInMaterial::set_alpha_threshold(f32 const threshold)
	{
		mAlphaThreshold = math::Clamp(threshold, 0, 1);
	}
}
