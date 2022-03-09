#include "Entity.hpp"

#include "../data/DataManager.hpp"
#include "../util/Logger.hpp"

#include <cstddef>


namespace leopph
{
	auto Entity::FindEntity(const std::string& name) -> Entity*
	{
		return internal::DataManager::Instance().FindEntity(name);
	}


	Entity::Entity(std::string name) :
		m_Name{
			[&]
			{
				if (!NameIsUnused(name))
				{
					name = GenerateUnusedName(name);
					internal::Logger::Instance().Warning("Name collision detected. Entity is being renamed to " + name + ".");
				}

				return name;
			}()
		}
	{
		auto& dataManager{internal::DataManager::Instance()};
		dataManager.RegisterEntity(this);
		m_Transform->Attach(this);
	}


	Entity::~Entity()
	{
		auto& dataManager{internal::DataManager::Instance()};

		Destroy(m_Transform); // Transform is a special case because it cannot be detached.

		for (const auto component : dataManager.ComponentsOfEntity(this, true))
		{
			component->Detach();
		}

		for (const auto component : dataManager.ComponentsOfEntity(this, false))
		{
			component->Detach();
		}

		dataManager.UnregisterEntity(this);
	}


	auto Entity::AttachComponent(Component* const component) -> void
	{
		component->Attach(this);
	}


	auto Entity::DetachComponent(Component* const component) const -> void
	{
		const auto& logger{internal::Logger::Instance()};

		if (!component)
		{
			logger.Warning("Ignoring attempt to remove nullptr component from Entity [" + m_Name + "].");
			return;
		}

		if (component->Entity() != this)
		{
			internal::Logger::Instance().Error("Ignoring attempt to remove component at [" + std::to_string(reinterpret_cast<std::size_t>(component)) + "] from Entity [" + m_Name + "], because the component is not owned by the Entity.");
			return;
		}

		component->Detach();
	}


	auto Entity::DeactiveAllComponents() const -> void
	{
		for (auto& component : internal::DataManager::Instance().ComponentsOfEntity(this, true))
		{
			component->Deactivate();
		}
	}


	auto Entity::ActivateAllComponents() const -> void
	{
		for (auto& component : internal::DataManager::Instance().ComponentsOfEntity(this, false))
		{
			component->Activate();
		}
	}


	auto Entity::Components() const -> std::span<Component* const>
	{
		return internal::DataManager::Instance().ComponentsOfEntity(this, true);
	}


	auto Entity::GenerateUnusedName(const std::string& namePrefix) -> std::string
	{
		static std::size_t entityCounter{0};
		auto newName{namePrefix + std::to_string(entityCounter)};
		++entityCounter;
		while (!NameIsUnused(newName))
		{
			newName = namePrefix + std::to_string(entityCounter);
			++entityCounter;
		}
		return newName;
	}


	auto Entity::NameIsUnused(const std::string& name) -> bool
	{
		return !internal::DataManager::Instance().FindEntity(name);
	}
}
