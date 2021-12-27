#include "SpotLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	SpotLight::SpotLight(leopph::Entity* const entity) :
		AttenuatedLight{entity},
		m_InnerAngle{30.f},
		m_OuterAngle{m_InnerAngle}
	{
		internal::DataManager::Instance().RegisterSpotLight(this);
	}

	SpotLight::~SpotLight()
	{
		internal::DataManager::Instance().UnregisterSpotLight(this);
	}

	auto SpotLight::InnerAngle() const -> float
	{
		return m_InnerAngle;
	}

	auto SpotLight::InnerAngle(const float degrees) -> void
	{
		m_InnerAngle = degrees;
	}

	auto SpotLight::OuterAngle() const -> float
	{
		return m_OuterAngle;
	}

	auto SpotLight::OuterAngle(const float degrees) -> void
	{
		m_OuterAngle = degrees;
	}
}
