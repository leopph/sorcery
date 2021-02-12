#include "object.h"
#include "instancedata.h"

#include <stdexcept>
#include <string>

namespace leopph
{
	// Constructor
	Object::Object():
		m_Name{ "Object" + std::to_string(implementation::InstanceData::Objects().size()) },
		m_Position{ Vector3{0.0f, 0.0f, 0.0f} },
		m_Rotation{ Vector3{0.0f, 0.0f, 0.0f} },
		m_Scale{ Vector3{1.0f, 1.0f, 1.0f} } {}






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

	void Object::UpdateAll()
	{
		for (auto& object : leopph::implementation::InstanceData::Objects())
			for (auto& behavior : object->Behaviors())
				behavior->operator()();
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
	const std::set<Behavior*>& Object::Behaviors() const
	{
		return m_Behaviors;
	}

	void Object::RemoveBehavior(Behavior* behavior)
	{
		m_Behaviors.erase(behavior);
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

	const Vector3& Object::Position() const
	{
		return m_Position;
	}

	void Object::Position(Vector3 newPos)
	{
		m_Position = std::move(newPos);
	}

	const Vector3& Object::Rotation() const
	{
		return m_Rotation;
	}

	void Object::Rotation(Vector3 newRot)
	{
		m_Rotation = std::move(newRot);
	}

	const Vector3& Object::Scale() const
	{
		return m_Scale;
	}

	void Object::Scale(Vector3 newScale)
	{
		m_Scale = std::move(newScale);
	}
}