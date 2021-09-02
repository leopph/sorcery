#include "Model.hpp"

#include "../data/DataManager.hpp"

#include <utility>

namespace leopph
{
	Model::Model(Object& owner, const std::filesystem::path& path) :
		Component{ owner }, ResourceHandle{ path }
	{
		impl::DataManager::RegisterModel(resource);
	}


	Model::~Model()
	{
		impl::DataManager::UnregisterModel(resource);
	}


	const std::filesystem::path& Model::Path() const
	{
		return resource->Path;
	}
}