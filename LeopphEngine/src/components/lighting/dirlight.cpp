#include "dirlight.h"

namespace leopph
{
	DirectionalLight::DirectionalLight()
	{
		RegisterDirectionalLight(this);
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