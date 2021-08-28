#include "Object.hpp"

#include "../instances/DataManager.hpp"

#include "../util/logger.h"

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace leopph
{
	Object* Object::Find(const std::string& name)
	{
		return impl::DataManager::FindObject(name);
	}


	Object::Object(const ObjectProperties& properties) :
		isStatic{ properties.isStatic },
		name { m_Name },
		m_Name{ properties.name.empty() ? "Object" + std::to_string(impl::DataManager::Objects().size()) : properties.name },
		m_Transform{ nullptr }
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
				throw std::runtime_error{ "Could not solve name conflict during creation of new object [" + m_Name + "]." };
			}
			impl::Logger::Instance().Warning("Object name [" + m_Name + "] is already taken. Renaming object to [" + newName + "]...");
			m_Name = newName;
		}

		impl::DataManager::RegisterObject(this);
		AddComponent<leopph::Transform>(properties.position, properties.rotation, properties.scale);
	}

	Object::Object(const bool isStatic) :
		Object{ isStatic, std::string{} }
	{}

	Object::Object(std::string name) :
		Object { false, std::move(name) }
	{}

	Object::Object() :
		Object{ false, std::string{} }
	{}

	Object::Object(const bool isStatic, std::string name) :
		Object{ ObjectProperties{ .name = std::move(name), .isStatic = isStatic } }
	{}



	Object::~Object()
	{
		for (auto it = impl::DataManager::Components(this).begin(); it != impl::DataManager::Components(this).end();)
		{
			delete* it;
			it = impl::DataManager::Components(this).begin();
		}

		impl::DataManager::UnregisterObject(this);
	}




	Transform& Object::Transform()
	{
		return const_cast<leopph::Transform&>(const_cast<const Object*>(this)->Transform());
	}

	const Transform& Object::Transform() const
	{
		if (m_Transform == nullptr)
		{
			m_Transform = GetComponent<leopph::Transform>();
		}

		return *m_Transform;
	}


	const std::set<Component*>& Object::Components() const
	{
		return impl::DataManager::Components(const_cast<Object*>(this));
	}


	void Object::RemoveComponent(Component* behavior)
	{
		if (&behavior->object != this)
		{
			const auto errorMsg{ "The given Component is not attached to Object [" + name + "]." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::invalid_argument{ errorMsg };
		}

		if (dynamic_cast<leopph::Transform*>(behavior))
		
		impl::DataManager::UnregisterComponent(behavior);
	}
}