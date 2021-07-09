#include "dirlight.h"
#include "../../instances/instanceholder.h"

namespace leopph
{
	DirectionalLight::DirectionalLight()
	{
		impl::InstanceHolder::DirectionalLight(this);
	}

	DirectionalLight::~DirectionalLight()
	{
		impl::InstanceHolder::DirectionalLight(nullptr);
	}

	const Vector3& DirectionalLight::Direction() const
	{
		return Light::Object().Transform().Forward();
	}
}