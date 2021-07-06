#include "modelreference.h"

namespace leopph::impl
{
	ModelReference::ModelReference(std::filesystem::path path) :
		m_ReferenceModel{ std::move(path) }
	{}

	void ModelReference::AddObject(Object* object)
	{
		m_Objects.insert(object);
	}

	void ModelReference::RemoveObject(Object* object)
	{
		m_Objects.erase(object);
	}

	std::size_t ModelReference::ReferenceCount() const
	{
		return m_Objects.size();
	}

	const AssimpModelImpl& ModelReference::ReferenceModel() const
	{
		return m_ReferenceModel;
	}

	const std::set<Object*> ModelReference::Objects() const
	{
		return m_Objects;
	}

}