#include "SpotLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	auto SpotLight::InnerAngle() const noexcept -> float
	{
		return m_InnerAngle;
	}


	auto SpotLight::InnerAngle(float const degrees) noexcept -> void
	{
		m_InnerAngle = degrees;
	}


	auto SpotLight::OuterAngle() const noexcept -> float
	{
		return m_OuterAngle;
	}


	auto SpotLight::OuterAngle(float const degrees) noexcept -> void
	{
		m_OuterAngle = degrees;
	}


	auto SpotLight::Owner(Entity* entity) -> void
	{
		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.RegisterActiveSpotLight(this);
		}

		AttenuatedLight::Owner(entity);

		if (InUse())
		{
			dataManager.UnregisterActiveSpotLight(this);
		}
	}


	auto SpotLight::Active(bool active) -> void
	{
		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.RegisterActiveSpotLight(this);
		}

		AttenuatedLight::Active(active);

		if (InUse())
		{
			dataManager.UnregisterActiveSpotLight(this);
		}
	}


	auto SpotLight::operator=(SpotLight const& other) -> SpotLight&
	{
		if (this == &other)
		{
			return *this;
		}

		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveSpotLight(this);
		}

		AttenuatedLight::operator=(other);
		m_InnerAngle = other.m_InnerAngle;
		m_OuterAngle = other.m_OuterAngle;

		if (InUse())
		{
			dataManager.RegisterActiveSpotLight(this);
		}

		return *this;
	}


	SpotLight::~SpotLight()
	{
		internal::DataManager::Instance().UnregisterActiveSpotLight(this);
	}
}
