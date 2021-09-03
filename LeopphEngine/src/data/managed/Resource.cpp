#include "Resource.hpp"

#include "../DataManager.hpp"


namespace leopph::impl
{
	Resource::Resource()
	{
		DataManager::Register(this);
	}


	Resource::~Resource()
	{
		DataManager::Unregister(this);
	}
}