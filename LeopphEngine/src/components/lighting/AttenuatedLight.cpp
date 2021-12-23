#include "AttenuatedLight.hpp"


namespace leopph::internal
{
	AttenuatedLight::AttenuatedLight(leopph::Entity* const entity, const float constant, const float linear, const float quadratic, const float range) :
		Light{entity},
		m_Constant{constant},
		m_Linear{linear},
		m_Quadratic{quadratic}
	{
		Range(range);
	}

	AttenuatedLight::~AttenuatedLight() = default;

	auto AttenuatedLight::Constant() const -> float
	{
		return m_Constant;
	}

	auto AttenuatedLight::Quadratic() const -> float
	{
		return m_Quadratic;
	}

	auto AttenuatedLight::Linear() const -> float
	{
		return m_Linear;
	}

	auto AttenuatedLight::Linear(const float value) -> void
	{
		m_Linear = value;
	}

	auto AttenuatedLight::Constant(const float value) -> void
	{
		m_Constant = value;
	}

	auto AttenuatedLight::Quadratic(const float value) -> void
	{
		m_Quadratic = value;
	}
}
