#include "Light.hpp"

namespace leopph::impl
{
	Light::Light(Entity& owner) :
		Component{ owner },
		m_Diffuse{ 1.0f, 1.0f, 1.0f },
		m_Specular{ 1.0f, 1.0f, 1.0f }
	{}

	Light::~Light() = default;


	const Vector3& Light::Diffuse() const
	{
		return m_Diffuse;
	}

	void Light::Diffuse(const Vector3& value)
	{
		m_Diffuse = value;
	}

	const Vector3& Light::Specular() const
	{
		return m_Specular;
	}

	void Light::Specular(const Vector3& value)
	{
		m_Specular = value;
	}
}