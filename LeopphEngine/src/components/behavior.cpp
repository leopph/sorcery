#include "Behavior.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
	Behavior::Behavior(leopph::Entity* const entity) :
		Component{entity}
	{
		impl::DataManager::Instance().RegisterBehavior(this);
	}

	Behavior::~Behavior()
	{
		impl::DataManager::Instance().UnregisterBehavior(this);
	}
}
