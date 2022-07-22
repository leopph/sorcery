#include "Light.hpp"


namespace leopph
{
	Vector3 const& Light::Diffuse() const noexcept
	{
		return m_Diffuse;
	}


	void Light::Diffuse(Vector3 const& value) noexcept
	{
		m_Diffuse = value;
	}


	Vector3 const& Light::Specular() const noexcept
	{
		return m_Specular;
	}


	void Light::Specular(Vector3 const& value) noexcept
	{
		m_Specular = value;
	}


	bool Light::CastsShadow() const noexcept
	{
		return m_CastsShadow;
	}


	void Light::CastsShadow(bool const value) noexcept
	{
		m_CastsShadow = value;
	}
}
