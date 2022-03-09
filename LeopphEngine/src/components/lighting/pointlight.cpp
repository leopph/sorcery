#include "PointLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	PointLight::PointLight()
	{
		internal::DataManager::Instance().RegisterActivePointLight(this);
	}


	auto PointLight::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		AttenuatedLight::Activate();

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterInactivePointLight(this);
		dataManager.RegisterActivePointLight(this);
	}


	auto PointLight::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		AttenuatedLight::Deactivate();

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterActivePointLight(this);
		dataManager.RegisterInactivePointLight(this);
	}


	PointLight::~PointLight()
	{
		if (IsActive())
		{
			internal::DataManager::Instance().UnregisterActivePointLight(this);
		}
		else
		{
			internal::DataManager::Instance().UnregisterInactivePointLight(this);
		}
	}
}
