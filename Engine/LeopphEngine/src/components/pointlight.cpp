#include "PointLight.hpp"

#include "Context.hpp"
#include "../rendering/Renderer.hpp"


namespace leopph
{
	PointLight::PointLight()
	{
		internal::get_renderer()->register_point_light(this);
	}



	PointLight::~PointLight()
	{
		internal::get_renderer()->unregister_point_light(this);
	}
}
