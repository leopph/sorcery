#include "Model.hpp"

#include "../data/DataManager.hpp"

#include <utility>

namespace leopph
{
	Model::Model(Object& owner, const std::filesystem::path& path) :
		Component{ owner }, ResourceHandle{ path }
	{
		impl::DataManager::Register(resource);
	}


	Model::~Model()
	{
		impl::DataManager::Unregister(resource);
	}


	const std::filesystem::path& Model::Path() const
	{
		return resource->Path;
	}
}