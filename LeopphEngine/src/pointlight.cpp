#include "pointlight.h"

namespace leopph
{
	PointLight::PointLight(Object& object) :
		Light{ object }
	{
		RegisterPointLight(this);
	}


	float PointLight::Range() const
	{
		// TODO
		return 0;
	}



	float PointLight::Constant() const
	{
		return m_Constant;
	}

	float PointLight::Linear() const
	{
		return m_Linear;
	}

	float PointLight::Quadratic() const
	{
		return m_Quadratic;
	}


	const Vector3& PointLight::Ambient() const
	{
		return m_Ambient;
	}

	const Vector3& PointLight::Diffuse() const
	{
		return m_Diffuse;
	}

	const Vector3& PointLight::Specular() const
	{
		return m_Specular;
	}



	void PointLight::Constant(float value)
	{
		m_Constant = value;
	}

	void PointLight::Linear(float value)
	{
		m_Linear = value;
	}

	void PointLight::Quadratic(float value)
	{
		m_Quadratic = value;
	}


	void PointLight::Ambient(const Vector3& value)
	{
		m_Ambient = value;
	}

	void PointLight::Diffuse(const Vector3& value)
	{
		m_Diffuse = value;
	}

	void PointLight::Specular(const Vector3& value)
	{
		m_Specular = value;
	}
}