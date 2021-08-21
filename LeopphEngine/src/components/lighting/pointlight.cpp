#include "PointLight.hpp"

#include "../../instances/InstanceHolder.hpp"

namespace leopph
{
	PointLight::PointLight(Object& owner) :
		AttenuatedLight{ owner }
	{
		impl::InstanceHolder::RegisterPointLight(this);
	}

	PointLight::~PointLight()
	{
		impl::InstanceHolder::UnregisterPointLight(this);
	}

}