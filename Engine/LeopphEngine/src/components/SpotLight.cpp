#include "SpotLight.hpp"

#include "DataManager.hpp"
#include "InternalContext.hpp"


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
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveSpotLight(this);
		}

		AttenuatedLight::Owner(entity);

		if (InUse())
		{
			dataManager->RegisterActiveSpotLight(this);
		}
	}



	auto SpotLight::Active(bool const active) -> void
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveSpotLight(this);
		}

		AttenuatedLight::Active(active);

		if (InUse())
		{
			dataManager->RegisterActiveSpotLight(this);
		}
	}



	auto SpotLight::Clone() const -> ComponentPtr<>
	{
		return CreateComponent<SpotLight>(*this);
	}



	auto SpotLight::operator=(SpotLight const& other) -> SpotLight&
	{
		if (this == &other)
		{
			return *this;
		}

		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveSpotLight(this);
		}

		AttenuatedLight::operator=(other);
		m_InnerAngle = other.m_InnerAngle;
		m_OuterAngle = other.m_OuterAngle;

		if (InUse())
		{
			dataManager->RegisterActiveSpotLight(this);
		}

		return *this;
	}



	SpotLight::~SpotLight()
	{
		internal::GetDataManager()->UnregisterActiveSpotLight(this);
	}
}
