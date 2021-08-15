#include "DirLight.hpp"

#include "../../instances/InstanceHolder.hpp"

namespace leopph
{
	DirectionalLight::DirectionalLight(Object& owner) :
		Light{ owner }
	{
		impl::InstanceHolder::DirectionalLight(this);
	}

	DirectionalLight::~DirectionalLight()
	{
		impl::InstanceHolder::DirectionalLight(nullptr);
	}

	const Vector3& DirectionalLight::Direction() const
	{
		return object.Transform().Forward();
	}
}