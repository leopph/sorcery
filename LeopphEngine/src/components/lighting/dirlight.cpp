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


	void DirectionalLight::Direction(const Vector3& newDir)
	{
		m_Direction = newDir;
	}

	const Vector3& DirectionalLight::Direction() const
	{
		return m_Direction;
	}
}