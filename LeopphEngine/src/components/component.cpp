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
			auto p = dataManager.UnregisterComponentFromEntity(m_Entity, this, false);
			dataManager.RegisterComponentForEntity(m_Entity, std::move(p), true);
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
			auto p = dataManager.UnregisterComponentFromEntity(m_Entity, this, true);
			dataManager.RegisterComponentForEntity(m_Entity, std::move(p), false);
		}
	}


	auto Component::Attach(leopph::Entity* entity) -> void
	{
		auto const& logger{internal::Logger::Instance()};

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

		try
		{
			internal::DataManager::Instance().RegisterComponentForEntity(entity, shared_from_this(), IsActive());
		}
		catch (std::bad_weak_ptr const&)
		{
			auto const errMsg{"Failed to attach Component because it is not a shared object."};
			logger.Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		m_Entity = entity;
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
