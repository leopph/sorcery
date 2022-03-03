#include "Component.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
	auto Component::Activate() -> void
	{
		if (m_IsActive)
		{
			return;
		}

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.RegisterActiveComponentForEntity(dataManager.UnregisterInactiveComponentFromEntity(this));
		m_IsActive = true;
	}


	auto Component::Deactivate() -> void
	{
		if (!m_IsActive)
		{
			return;
		}

		auto& dataManager{internal::DataManager::Instance()};
		dataManager.RegisterInactiveComponentForEntity(dataManager.UnregisterActiveComponentFromEntity(this));
		m_IsActive = false;
	}


	Component::Component(leopph::Entity* const entity) :
		m_Entity{entity}
	{}
}
