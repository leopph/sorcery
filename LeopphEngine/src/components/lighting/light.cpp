#include "Light.hpp"



namespace leopph::impl
{
	Light::Light(Entity& owner) :
		Component{owner},
		m_CastsShadow{false},
		m_Range{1000.f},
		m_Diffuse{1.0f, 1.0f, 1.0f},
		m_Specular{1.0f, 1.0f, 1.0f}
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


	float Light::Range() const
	{
		return m_Range;
	}


	void Light::Range(const float value)
	{
		m_Range = value;
	}


	bool Light::CastsShadow() const
	{
		return m_CastsShadow;
	}


	void Light::CastsShadow(const bool value)
	{
		m_CastsShadow = value;
	}

}
