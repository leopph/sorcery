#include "pointlight.h"

namespace leopph
{
	PointLight::PointLight(leopph::Object& object) :
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
}