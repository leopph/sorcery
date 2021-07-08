#include "object.h"

#include "../instances/instanceholder.h"

#include "../util/logger.h"

#include <utility>
#include <stdexcept>
#include <string>

namespace leopph
{
	Object* Object::Create()
	{
		const auto ret = new Object;
		impl::InstanceHolder::StoreObject(ret);
		return ret;
	}

	void Object::Destroy(Object*& object)
	{
		impl::InstanceHolder::DeleteObject(object);
		object = nullptr;
	}

	Object* Object::Find(const std::string& name)
	{
		return impl::InstanceHolder::FindObject(name);
	}


	
	Object::Object():
		m_Name{ "Object" + std::to_string(impl::InstanceHolder::Objects().size()) }
	{}



	leopph::Transform& Object::Transform()
	{
		return const_cast<leopph::Transform&>(const_cast<const Object*>(this)->Transform());
	}

	const leopph::Transform& Object::Transform() const
	{
		return m_Transform;
	}

	const std::string& Object::Name() const
	{
		return m_Name;
	}

	void Object::Name(const std::string& newName)
	{		
		if (impl::InstanceHolder::FindObject(newName) != nullptr)
		{
			const auto errorMsg{ "Object [" + newName + "] already exists." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::invalid_argument{ errorMsg };
		}
		
		impl::InstanceHolder::RenameObject(this, newName);
	}


	const std::set<Component*>& Object::Components() const
	{
		return impl::InstanceHolder::Components(const_cast<Object*>(this));
	}


	void Object::RemoveComponent(Component* behavior)
	{
		if (&behavior->Object() != this)
		{
			const auto errorMsg{ "The given Component is not attached to Object [" + Name() + "]." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::invalid_argument{ errorMsg };
		}
		
		impl::InstanceHolder::UnregisterComponent(behavior);
	}
}