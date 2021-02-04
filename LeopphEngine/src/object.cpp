#include "object.h"

#include <stdexcept>
#include <string>

namespace leopph
{
	// Init static container
	std::set<std::unique_ptr<Object, Object::Deleter>, Object::Comparator> Object::s_Instances{};





	// Deleter
	void Object::Deleter::operator()(Object* object) const
	{
		delete object;
	}




	// Comparisons for name-based lookups
	bool Object::Comparator::operator()(const std::unique_ptr<Object, Deleter>& left, const std::unique_ptr<Object, Deleter>& right) const
	{
		return left->m_Name < right->m_Name;
	}

	bool Object::Comparator::operator()(const std::unique_ptr<Object, Deleter>& left, const std::string& right) const
	{
		return left->m_Name < right;
	}

	bool Object::Comparator::operator()(const std::string& left, const std::unique_ptr<Object, Deleter>& right) const
	{
		return left < right->m_Name;
	}

	bool Object::Comparator::operator()(const std::unique_ptr<Object, Deleter>& left, const Object* right) const
	{
		return left->m_Name < right->m_Name;
	}

	bool Object::Comparator::operator()(const Object* left, const std::unique_ptr<Object, Deleter>& right) const
	{
		return left->m_Name < right->m_Name;
	}


	


	// Constructor
	Object::Object():
		m_Name{ "Object" + std::to_string(s_Instances.size()) },
		m_Position{ Vector3{0.0f, 0.0f, 0.0f} },
		m_Rotation{ Vector3{0.0f, 0.0f, 0.0f} },
		m_Scale{ Vector3{1.0f, 1.0f, 1.0f} } {}






	// Static functions
	Object* Object::Create()
	{
		return s_Instances.emplace(new Object).first->get();
	}

	void Object::Destroy(Object*& object)
	{
		s_Instances.erase(s_Instances.find(object));
		object = nullptr;
	}

	Object* Object::Find(const std::string& name)
	{
		auto it = s_Instances.find(name);

		return it != s_Instances.end() ? it->get() : nullptr;
	}

	const std::set<std::unique_ptr<Object, Object::Deleter>, Object::Comparator>& Object::Instances()
	{
		return s_Instances;
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

		std::string oldName = m_Name;
		auto node = s_Instances.extract(s_Instances.find(this));
		m_Name = std::move(newName);
		auto result = s_Instances.insert(std::move(node));

		if (!result.inserted)
		{
			m_Name = std::move(oldName);
			s_Instances.insert(std::move(result.node));
		}
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