#include "DirectionalLight.hpp"

#include "Context.hpp"
#include "../../../include/Entity.hpp"
#include "../rendering/Renderer.hpp"


namespace leopph
{
	Vector3 const& DirectionalLight::get_direction() const
	{
		return get_owner()->get_forward_axis();
	}



	f32 DirectionalLight::get_shadow_near_plane() const
	{
		return mShadowNear;
	}



	void DirectionalLight::set_shadow_near_plane(f32 const newVal)
	{
		mShadowNear = newVal;
	}



	DirectionalLight::DirectionalLight()
	{
		internal::get_renderer()->register_dir_light(this);
	}



	DirectionalLight::~DirectionalLight()
	{
		internal::get_renderer()->unregister_dir_light(this);
	}
}
