#include "UniqueResource.hpp"

#include "../DataManager.hpp"

#include <utility>


namespace leopph::impl
{
	UniqueResource::UniqueResource(std::filesystem::path path) :
		Path{ std::move(path) }
	{
		DataManager::RegisterResource(this);
	}


	UniqueResource::~UniqueResource()
	{
		DataManager::UnregisterResource(this);
	}
}
