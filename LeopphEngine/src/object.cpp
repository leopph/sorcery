#include "object.h"
#include "instancedata.h"

#include <stdexcept>
#include <string>

namespace leopph
{
	// Constructor
	Object::Object():
		m_Name{ "Object" + std::to_string(implementation::InstanceData::Objects().size()) }
	{}





	leopph::Transform& Object::Transform()
	{
		return const_cast<leopph::Transform&>(const_cast<const Object*>(this)->Transform());
	}

	const leopph::Transform& Object::Transform() const
	{
		return m_Transform;
	}





	// Static functions
	Object* Object::Create()
	{
		Object* ret = new Object;
		implementation::InstanceData::AddObject(ret, [](Object* object) { delete object; });
		return ret;
	}

	void Object::Destroy(Object*& object)
	{
		implementation::InstanceData::RemoveObject(object);
		object = nullptr;
	}

	Object* Object::Find(const std::string& name)
	{
		return implementation::InstanceData::FindObject(name);
	}





	// Models
	const std::vector<Model>& Object::Models() const
	{
		return m_Models;
	}

	void Object::AddModel(Model&& model)
	{
		m_Models.emplace_back(std::move(model));
	}

	void Object::RemoveModel(size_t index)
	{
		if (m_Models.size() <= index || index < 0)
			throw std::exception{};
	}





	// Behaviors
	const std::set<Component*>& Object::Components() const
	{
		return m_Components;
	}

	void Object::RemoveComponent(Behavior* behavior)
	{
		m_Components.erase(behavior);
	}






	// Nonstatic members
	const std::string& Object::Name() const
	{
		return m_Name;
	}

	void Object::Name(std::string newName)
	{
		/* Removing the node is necessary, because m_Name is the ordering key.
		If the target node is not removed before renaming, order-changing names may lead to crashes.
		In case the new name is already in use, revert to the previous name and reinsert the node without changes. */

		implementation::InstanceData::UpdateObjectKey(std::move(m_Name), std::move(newName), [](Object* object, std::string&& newName) { object->m_Name = std::move(newName); });
	}
}