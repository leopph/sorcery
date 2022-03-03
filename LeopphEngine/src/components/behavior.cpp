#include "Behavior.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
	Behavior::Behavior(leopph::Entity* const entity) :
		Component{entity}
	{
		internal::DataManager::Instance().RegisterActiveBehavior(this);
	}


	auto Behavior::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		Component::Activate();

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterActiveBehavior(this);
		dataManager.RegisterInactiveBehavior(this);
	}


	auto Behavior::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		Component::Deactivate();

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterInactiveBehavior(this);
		dataManager.RegisterActiveBehavior(this);
	}


	Behavior::~Behavior()
	{
		if (IsActive())
		{
			internal::DataManager::Instance().UnregisterActiveBehavior(this);
		}
		else
		{
			internal::DataManager::Instance().UnregisterInactiveBehavior(this);
		}
	}
}
