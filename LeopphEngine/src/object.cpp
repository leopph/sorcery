#include "object.h"

#include <stdexcept>
#include <string>

namespace leopph
{
	std::set<Object*> Object::s_Instances{};


	Object::Object()
	{
		s_Instances.insert(this);
	}


	Object::~Object()
	{
		s_Instances.erase(this);
	}


	const std::set<Object*>& Object::Instances()
	{
		return s_Instances;
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


	void Object::Update() {}

	
	const glm::vec3& Object::Position() const
	{
		return m_Position;
	}

	void Object::Position(glm::vec3 newPos)
	{
		m_Position = newPos;
	}

	const glm::vec3& Object::Rotation() const
	{
		return m_Rotation;
	}

	void Object::Rotation(glm::vec3 newRot)
	{
		m_Rotation = newRot;
	}

	const glm::vec3& Object::Scale() const
	{
		return m_Scale;
	}

	void Object::Scale(glm::vec3 newScale)
	{
		m_Scale = newScale;
	}
}