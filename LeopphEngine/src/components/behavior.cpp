#include "Behavior.hpp"

#include "../instances/DataManager.hpp"

namespace leopph
{
	Behavior::Behavior(Object& owner) :
		Component{ owner }
	{
		impl::DataManager::RegisterBehavior(this);
	}

	Behavior::~Behavior()
	{
		impl::DataManager::UnregisterBehavior(this);
	}
}