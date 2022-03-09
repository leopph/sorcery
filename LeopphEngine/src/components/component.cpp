#include "Component.hpp"

#include "../data/DataManager.hpp"
#include "../util/Logger.hpp"


namespace leopph
{
	auto Component::Activate() -> void
	{
		if (m_IsActive)
		{
			return;
		}

		m_IsActive = true;

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterComponentFromEntity(m_Entity, this, false);
			dataManager.RegisterComponentForEntity(m_Entity, this, true);
		}
	}


	auto Component::Deactivate() -> void
	{
		if (!m_IsActive)
		{
			return;
		}

		m_IsActive = false;

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterComponentFromEntity(m_Entity, this, true);
			dataManager.RegisterComponentForEntity(m_Entity, this, false);
		}
	}


	auto Component::Attach(leopph::Entity* entity) -> void
	{
		const auto& logger{internal::Logger::Instance()};

		if (!entity)
		{
			logger.Warning("Ignoring attempt to attach Component to nullptr.");
			return;
		}

		if (IsAttached())
		{
			if (m_Entity == entity)
			{
				logger.Warning("Ignoring attempt to reattach Component to Entity [" + entity->Name() + "].");
				return;
			}

			Detach();
		}

		m_Entity = entity;
		internal::DataManager::Instance().RegisterComponentForEntity(m_Entity, this, IsActive());
	}


	auto Component::Detach() -> void
	{
		if (!IsAttached())
		{
			return;
		}

		internal::DataManager::Instance().UnregisterComponentFromEntity(m_Entity, this, IsActive());
		m_Entity = nullptr;
	}


	auto Component::IsAttached() const -> bool
	{
		return m_Entity != nullptr;
	}


	Component::~Component()
	{
		Component::Detach();
	}
}
