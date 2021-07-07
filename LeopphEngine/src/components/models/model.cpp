#include "model.h"

#include "../../rendering/assimpmodel.h"
#include "../../instances/instanceholder.h"

#include <utility>

namespace leopph
{
	Model::Model(std::filesystem::path path) :
		m_Ref{ &impl::InstanceHolder::GetModelReference(path) }
	{}

	void Model::Init()
	{
		impl::InstanceHolder::RegisterModelObject(Path(), &Component::Object());
	}


	Model::~Model()
	{
		impl::InstanceHolder::UnregisterModelObject(Path(), &Component::Object());
	}


	bool Model::operator==(const Model& other) const
	{
		return *m_Ref == *other.m_Ref;
	}


	const std::filesystem::path& Model::Path() const
	{
		return m_Ref->Path();
	}
}