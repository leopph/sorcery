#include "PointLight.hpp"

#include "../InternalContext.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	void PointLight::Owner(Entity* entity)
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActivePointLight(this);
		}

		AttenuatedLight::Owner(entity);

		if (InUse())
		{
			dataManager->RegisterActivePointLight(this);
		}
	}



	void PointLight::Active(bool const active)
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActivePointLight(this);
		}

		AttenuatedLight::Active(active);

		if (InUse())
		{
			dataManager->RegisterActivePointLight(this);
		}
	}



	PointLight& PointLight::operator=(PointLight const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActivePointLight(this);
		}

		AttenuatedLight::operator=(other);

		if (InUse())
		{
			dataManager->RegisterActivePointLight(this);
		}

		return *this;
	}



	ComponentPtr<> PointLight::Clone() const
	{
		return CreateComponent<PointLight>(*this);
	}



	PointLight::~PointLight()
	{
		internal::GetDataManager()->UnregisterActivePointLight(this);
	}
}
