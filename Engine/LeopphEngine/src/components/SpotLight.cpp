#include "SpotLight.hpp"

#include "Context.hpp"
#include "../rendering/Renderer.hpp"


namespace leopph
{
	float SpotLight::get_inner_angle() const
	{
		return mInnerAngle;
	}



	void SpotLight::set_inner_angle(f32 const degrees)
	{
		mInnerAngle = degrees;
	}



	float SpotLight::get_outer_angle() const
	{
		return mOuterAngle;
	}



	void SpotLight::set_outer_angle(f32 const degrees)
	{
		mOuterAngle = degrees;
	}



	SpotLight::SpotLight()
	{
		internal::get_renderer().register_spot_light(this);
	}



	SpotLight::~SpotLight()
	{
		internal::get_renderer().unregister_spot_light(this);
	}
}
