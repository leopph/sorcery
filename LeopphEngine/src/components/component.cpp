#include "Component.hpp"

#include "../data/DataManager.hpp"

namespace leopph
{
	Component::Component(Entity& owner) :
		entity{ owner }
	{
		impl::DataManager::Register(this);
	}

	Component::~Component()
	{
		impl::DataManager::Unregister(this);
	}
}