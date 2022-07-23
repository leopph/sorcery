#include "SpotLight.hpp"

#include "../InternalContext.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	float SpotLight::InnerAngle() const noexcept
	{
		return m_InnerAngle;
	}



	void SpotLight::InnerAngle(float const degrees) noexcept
	{
		m_InnerAngle = degrees;
	}



	float SpotLight::OuterAngle() const noexcept
	{
		return m_OuterAngle;
	}



	void SpotLight::OuterAngle(float const degrees) noexcept
	{
		m_OuterAngle = degrees;
	}



	void SpotLight::Owner(Entity* entity)
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



	void SpotLight::Active(bool const active)
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



	ComponentPtr<> SpotLight::Clone() const
	{
		return CreateComponent<SpotLight>(*this);
	}



	SpotLight& SpotLight::operator=(SpotLight const& other)
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
