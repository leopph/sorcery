#include "SpotLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	SpotLight::SpotLight(leopph::Entity* const entity) :
		AttenuatedLight{entity, 1.f, 0.1f, 0.1f},
		m_InnerAngle{30.f},
		m_OuterAngle{m_InnerAngle}
	{
		impl::DataManager::Instance().RegisterSpotLight(this);
	}

	SpotLight::~SpotLight()
	{
		impl::DataManager::Instance().UnregisterSpotLight(this);
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
