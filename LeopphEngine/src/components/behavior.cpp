#include "Behavior.hpp"

#include "../data/DataManager.hpp"

namespace leopph
{
	Behavior::Behavior(Object& owner) :
		Component{ owner }
	{
		impl::DataManager::Register(this);
	}

	Behavior::~Behavior()
	{
		impl::DataManager::Unregister(this);
	}
}