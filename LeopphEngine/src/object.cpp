#include "object.h"

#include <stdexcept>
#include <string>

namespace leopph
{

	// Comparisons for name-based lookups
	bool Object::Compare::operator()(const Object* left, const Object* right) const
	{
		return left->m_Name < right->m_Name;
	}

	bool Object::Compare::operator()(const Object* left, const std::string& right) const
	{
		return left->m_Name < right;
	}

	bool Object::Compare::operator()(const std::string& left, const Object* right) const
	{
		return left < right->m_Name;
	}



	std::set<Object*, Object::Compare> Object::s_Instances{};

	

	Object::Object()
		: m_Name{ "Object" + std::to_string(s_Instances.size()) }
	{
		s_Instances.insert(this);
	}

	Object::~Object()
	{
		for (Behavior* behavior : m_Behaviors)
			delete behavior;

		s_Instances.erase(this);
	}



	Object* Object::Get(const std::string& name)
	{
		auto iterator{ s_Instances.find(name) };

		return iterator == s_Instances.end() ? nullptr : *iterator;
	}



	const std::set<Object*, Object::Compare>& Object::Instances()
	{
		return s_Instances;
	}

	const std::set<Behavior*>& Object::Behaviors() const
	{
		return m_Behaviors;
	}

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



	template<class T>
	Behavior* Object::AddBehavior()
	{
		m_Behaviors.emplace(new T);
	}

	void Object::RemoveBehavior(Behavior* behavior)
	{
		m_Behaviors.erase(behavior);
	}

	template<class T>
	Behavior* Object::GetBehavior() const
	{
		// TODO
	}



	const std::string& Object::Name() const
	{
		return m_Name;
	}

	void Object::Name(std::string newName)
	{
		m_Name = std::move(newName);
	}

	
	const glm::vec3& Object::Position() const
	{
		return m_Position;
	}

	void Object::Position(glm::vec3 newPos)
	{
		m_Position = std::move(newPos);
	}



	const glm::vec3& Object::Rotation() const
	{
		return m_Rotation;
	}

	void Object::Rotation(glm::vec3 newRot)
	{
		m_Rotation = std::move(newRot);
	}



	const glm::vec3& Object::Scale() const
	{
		return m_Scale;
	}

	void Object::Scale(glm::vec3 newScale)
	{
		m_Scale = std::move(newScale);
	}
}