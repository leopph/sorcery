#include "Behavior.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
	Behavior::Behavior(leopph::Entity* const entity) :
		Component{entity}
	{
		internal::DataManager::Instance().RegisterBehavior(this);
	}

	Behavior::~Behavior()
	{
		internal::DataManager::Instance().UnregisterBehavior(this);
	}
}
