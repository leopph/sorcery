#include "Model.hpp"

#include "../../rendering/assimpmodel.h"
#include "../../data/DataManager.hpp"

#include <utility>

namespace leopph
{
	Model::Model(Object& owner, const std::filesystem::path& path) :
		Component{ owner }, m_Ref{ &impl::DataManager::GetModelReference(path) }
	{
		impl::DataManager::IncModel(Path(), &object);
	}


	Model::~Model()
	{
		impl::DataManager::DecModel(Path(), &object);
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