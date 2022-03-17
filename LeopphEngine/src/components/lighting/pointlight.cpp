#include "PointLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	auto PointLight::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		AttenuatedLight::Activate();

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterPointLight(this, false);
			dataManager.RegisterPointLight(this, true);
		}
	}


	auto PointLight::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		AttenuatedLight::Deactivate();

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterPointLight(this, true);
			dataManager.RegisterPointLight(this, false);
		}
	}


	auto PointLight::Attach(leopph::Entity* entity) -> void
	{
		if (IsAttached())
		{
			return;
		}

		AttenuatedLight::Attach(entity);

		internal::DataManager::Instance().RegisterPointLight(this, IsActive());
	}


	auto PointLight::Detach() -> void
	{
		if (!IsAttached())
		{
			return;
		}

		AttenuatedLight::Detach();

		internal::DataManager::Instance().UnregisterPointLight(this, IsActive());
	}


	PointLight::~PointLight()
	{
		Detach();
	}
}
