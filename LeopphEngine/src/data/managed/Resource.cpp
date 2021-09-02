#include "Resource.hpp"

#include "../DataManager.hpp"


namespace leopph::impl
{
	Resource::Resource()
	{
		DataManager::RegisterResource(this);
	}


	Resource::~Resource()
	{
		DataManager::UnregisterResource(this);
	}
}