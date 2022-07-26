#include "SpotLight.hpp"

#include "../InternalContext.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	float SpotLight::get_inner_angle() const
	{
		return mInnerAngle;
	}



	void SpotLight::set_inner_angle(f32 const degrees)
	{
		mInnerAngle = degrees;
	}



	float SpotLight::get_outer_angle() const
	{
		return mOuterAngle;
	}



	void SpotLight::set_outer_angle(f32 const degrees)
	{
		mOuterAngle = degrees;
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
		mInnerAngle = other.mInnerAngle;
		mOuterAngle = other.mOuterAngle;

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
