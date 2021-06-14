#include "object.h"
#include "../instances/instancedata.h"
#include <stdexcept>
#include <string>

namespace leopph
{
	Object* Object::Create()
	{
		const auto ret = new Object;
		impl::InstanceData::AddObject(ret);
		return ret;
	}

	void Object::Destroy(Object*& object)
	{
		impl::InstanceData::RemoveObject(object);
		object = nullptr;
	}

	Object* Object::Find(const std::string& name)
	{
		return impl::InstanceData::FindObject(name);
	}


	
	Object::Object():
		m_Name{ "Object" + std::to_string(impl::InstanceData::Objects().size()) }
	{}

	Object::~Object()
	{
		for (Component* component : m_Components)
			delete component;
	}



	leopph::Transform& Object::Transform()
	{
		return const_cast<leopph::Transform&>(const_cast<const Object*>(this)->Transform());
	}

	const leopph::Transform& Object::Transform() const
	{
		return m_Transform;
	}

	const std::unordered_set<Model>& Object::Models() const
	{
		return m_Models;
	}

	const Model* Object::AddModel(std::filesystem::path path)
	{
		const auto insertionData= m_Models.emplace(path);

		if (!insertionData.second)
			throw std::runtime_error{ "Model was not inserted due to an error!" };
		
		return &*insertionData.first;
	}

	void Object::RemoveModel(const Model* & model)
	{
		/* erase returns 0-1 depending on whether the element was removed */
		if (m_Models.erase(*model) == 1)
			model = nullptr;
	}

	const std::set<Component*>& Object::Components() const
	{
		return m_Components;
	}
	
	void Object::RemoveComponent(Component* behavior)
	{
		m_Components.erase(behavior);
		delete behavior;
	}

	const std::string& Object::Name() const
	{
		return m_Name;
	}

	void Object::Name(const std::string& newName)
	{
		/* Removing the node is necessary, because m_Name is the ordering key. */
		
		if (impl::InstanceData::FindObject(newName) != nullptr)
			throw std::invalid_argument{ "Object [" + newName + "] already exists!" };
		
		impl::InstanceData::RemoveObject(this);
		m_Name = newName;
		impl::InstanceData::AddObject(this);
	}
}