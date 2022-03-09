#include "Behavior.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
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


	auto Behavior::Attach(leopph::Entity* entity) -> void
	{
		Component::Attach(entity);

		if (IsActive())
		{
			internal::DataManager::Instance().RegisterActiveBehavior(this);
		}
		else
		{
			internal::DataManager::Instance().RegisterInactiveBehavior(this);
		}
	}


	auto Behavior::Detach() -> void
	{
		Component::Detach();

		if (IsActive())
		{
			internal::DataManager::Instance().UnregisterActiveBehavior(this);
		}
		else
		{
			internal::DataManager::Instance().UnregisterInactiveBehavior(this);
		}
	}


	Behavior::~Behavior()
	{
		Behavior::Detach();
	}
}
