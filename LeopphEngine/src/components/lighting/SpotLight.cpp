#include "SpotLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	auto SpotLight::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		AttenuatedLight::Activate();

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterInactiveSpotLight(this);
		dataManager.RegisterActiveSpotLight(this);
	}


	auto SpotLight::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		AttenuatedLight::Deactivate();

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterActiveSpotLight(this);
		dataManager.RegisterInactiveSpotLight(this);
	}


	SpotLight::SpotLight()
	{
		internal::DataManager::Instance().RegisterActiveSpotLight(this);
	}


	SpotLight::~SpotLight()
	{
		if (IsActive())
		{
			internal::DataManager::Instance().UnregisterActiveSpotLight(this);
		}
		else
		{
			internal::DataManager::Instance().UnregisterInactiveSpotLight(this);
		}
	}
}
