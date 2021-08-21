#include "SpotLight.hpp"

#include "../../instances/InstanceHolder.hpp"

namespace leopph
{
	SpotLight::SpotLight(Object& owner) :
		AttenuatedLight{ owner, 1.f, 0.1f, 0.1f }, m_Angle{ 30.f }
	{
		impl::InstanceHolder::RegisterSpotLight(this);
	}

	SpotLight::~SpotLight()
	{
		impl::InstanceHolder::UnregisterSpotLight(this);
	}

	float SpotLight::Angle() const
	{
		return m_Angle;
	}

	void SpotLight::Angle(const float degrees)
	{
		m_Angle = degrees;
	}

}