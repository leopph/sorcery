#include "Entity.hpp"

#include "../data/DataManager.hpp"
#include "../util/logger.h"

#include <iterator>
#include <limits>
#include <stdexcept>


namespace leopph
{
	Entity* Entity::Find(const std::string& name)
	{
		return internal::DataManager::Instance().FindEntity(name);
	}


	Entity::Entity(std::string name) :
		m_Name{name.empty() ? "Entity" + std::to_string(internal::DataManager::Instance().EntitiesAndComponents().size()) : std::move(name)}
	{
		if (Find(this->m_Name) != nullptr)
		{
			std::string newName{};
			for (std::size_t i = 0; i < std::numeric_limits<std::size_t>::max(); i++)
			{
				newName = m_Name + "(" + std::to_string(i) + ")";
				if (Find(newName) == nullptr)
				{
					break;
				}
			}
			if (newName.empty())
			{
				const auto errMsg{"Could not solve name conflict during creation of new Entity [" + m_Name + "]."};
				internal::Logger::Instance().Critical(errMsg);
				throw std::invalid_argument{errMsg};
			}
			internal::Logger::Instance().Warning("Entity name [" + m_Name + "] is already taken. Renaming Entity to [" + newName + "]...");
			m_Name = newName;
		}

		internal::DataManager::Instance().RegisterEntity(this);
		m_Transform = CreateComponent<leopph::Transform>();
	}


	Entity::Entity() :
		Entity{std::string{}}
	{}


	Entity::~Entity() noexcept
	{
		internal::DataManager::Instance().UnregisterEntity(this);
	}


	std::vector<Component*> Entity::Components() const
	{
		const auto& components{internal::DataManager::Instance().ComponentsOfEntity(this)};
		std::vector<Component*> ret(components.size());
		std::ranges::transform(components, std::back_inserter(ret), [](const auto& compPtr)
		{
			return compPtr.get();
		});
		return ret;
	}


	void Entity::RemoveComponent(const Component* component) const
	{
		if (component->Entity() != this)
		{
			const auto msg{"Error while trying to remove Component at address [" + std::to_string(reinterpret_cast<unsigned long long>(component)) + "] from Entity [" + m_Name + "]. The Component's owning Entity is different from this."};
			internal::Logger::Instance().Error(msg);
		}
		else
		{
			internal::DataManager::Instance().UnregisterComponentFromEntity(this, component);
		}
	}

	void Entity::RegisterComponent(std::unique_ptr<Component>&& component) const
	{
		internal::DataManager::Instance().RegisterComponentForEntity(this, std::move(component));
	}

	const std::string& Entity::Name() const
	{
		return m_Name;
	}

	Transform* Entity::Transform() const
	{
		return m_Transform;
	}
}
