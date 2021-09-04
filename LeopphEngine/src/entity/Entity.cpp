#include "Entity.hpp"

#include "../data/DataManager.hpp"

#include "../util/logger.h"

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>


namespace leopph
{
	Entity* Entity::Find(const std::string& name)
	{
		return impl::DataManager::Find(name);
	}


	Entity::Entity(std::string name) :
		name{m_Name},
		m_Name{name.empty() ? "Entity" + std::to_string(impl::DataManager::EntitiesAndComponents().size()) : name},
		m_Transform{nullptr}
	{
		if (Find(this->name) != nullptr)
		{
			std::string newName{};
			for (std::size_t i = 0; i < std::numeric_limits<std::size_t>::max(); i++)
			{
				newName = m_Name + "(" + std::to_string(i) + ")";
				if (Find(newName) == nullptr)
					break;
			}
			if (newName.empty())
			{
				throw std::runtime_error{"Could not solve name conflict during creation of new Entity [" + m_Name + "]."};
			}
			impl::Logger::Instance().Warning("Entity name [" + m_Name + "] is already taken. Renaming Entity to [" + newName + "]...");
			m_Name = newName;
		}

		impl::DataManager::Register(this);
		AddComponent<leopph::Transform>();
	}


	Entity::Entity() :
		Entity{std::string{}}
	{
	}


	Entity::~Entity()
	{
		for (auto it = impl::DataManager::Components(this).begin(); it != impl::DataManager::Components(this).end();)
		{
			delete* it;
			it = impl::DataManager::Components(this).begin();
		}

		impl::DataManager::Unregister(this);
	}


	Transform& Entity::Transform()
	{
		return const_cast<leopph::Transform&>(const_cast<const Entity*>(this)->Transform());
	}


	const Transform& Entity::Transform() const
	{
		if (m_Transform == nullptr)
		{
			m_Transform = GetComponent<leopph::Transform>();
		}

		return *m_Transform;
	}


	const std::unordered_set<Component*>& Entity::Components() const
	{
		return impl::DataManager::Components(const_cast<Entity*>(this));
	}


	void Entity::RemoveComponent(Component* behavior)
	{
		if (&behavior->entity != this)
		{
			const auto errorMsg{"The given Component is not attached to Entity [" + name + "]."};
			impl::Logger::Instance().Error(errorMsg);
			throw std::invalid_argument{errorMsg};
		}

		if (dynamic_cast<leopph::Transform*>(behavior))
		{
			impl::DataManager::Unregister(behavior);
		}
	}
}