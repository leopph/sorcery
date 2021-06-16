#include "light.h"

namespace leopph::impl
{
	Light::~Light() = default;

	
	const Vector3& Light::Ambient() const
	{
		return m_Ambient;
	}

	const Vector3& Light::Diffuse() const
	{
		return m_Diffuse;
	}

	const Vector3& Light::Specular() const
	{
		return m_Specular;
	}

	void Light::Ambient(const Vector3& value)
	{
		m_Ambient = value;
	}

	void Light::Diffuse(const Vector3& value)
	{
		m_Diffuse = value;
	}

	void Light::Specular(const Vector3& value)
	{
		m_Specular = value;
	}
}