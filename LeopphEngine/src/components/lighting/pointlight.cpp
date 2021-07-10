#include "pointlight.h"
#include "../../instances/instanceholder.h"

namespace leopph
{
	PointLight::PointLight() :
		m_Constant{ 1.0f }, m_Linear{ 0.14f }, m_Quadratic{ 0.07f }
	{
		impl::InstanceHolder::RegisterPointLight(this);
	}

	PointLight::~PointLight()
	{
		impl::InstanceHolder::UnregisterPointLight(this);
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