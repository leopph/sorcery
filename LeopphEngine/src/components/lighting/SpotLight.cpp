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

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterSpotLight(this, false);
			dataManager.RegisterSpotLight(this, true);
		}
	}


	auto SpotLight::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		AttenuatedLight::Deactivate();

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterSpotLight(this, true);
			dataManager.RegisterSpotLight(this, false);
		}
	}


	auto SpotLight::Attach(leopph::Entity* entity) -> void
	{
		if (IsAttached())
		{
			return;
		}

		AttenuatedLight::Attach(entity);

		internal::DataManager::Instance().RegisterSpotLight(this, IsActive());
	}


	auto SpotLight::Detach() -> void
	{
		if (!IsAttached())
		{
			return;
		}

		AttenuatedLight::Detach();

		internal::DataManager::Instance().UnregisterSpotLight(this, IsActive());
	}


	SpotLight::~SpotLight()
	{
		Detach();
	}
}
