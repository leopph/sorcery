#include "SpotLight.hpp"

#include "../../instances/InstanceHolder.hpp"

namespace leopph
{
	SpotLight::SpotLight(Object& owner) :
		AttenuatedLight{ owner, 1.f, 0.1f, 0.1f }, m_InnerAngle{ 30.f }, m_OuterAngle{ m_InnerAngle }
	{
		impl::InstanceHolder::RegisterSpotLight(this);
	}

	SpotLight::~SpotLight()
	{
		impl::InstanceHolder::UnregisterSpotLight(this);
	}

	float SpotLight::InnerAngle() const
	{
		return m_InnerAngle;
	}

	void SpotLight::InnerAngle(const float degrees)
	{
		m_InnerAngle = degrees;
	}

	float SpotLight::OuterAngle() const
	{
		return m_OuterAngle;
	}

	void SpotLight::OuterAngle(const float degrees)
	{
		m_OuterAngle = degrees;
	}

}