#include "Behavior.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
	Behavior::Behavior(leopph::Entity* const entity) :
		Component{entity}
	{
		impl::DataManager::Register(this);
	}

	Behavior::~Behavior()
	{
		impl::DataManager::Unregister(this);
	}
}
