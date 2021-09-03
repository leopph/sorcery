#include "Component.hpp"

#include "../data/DataManager.hpp"

namespace leopph
{
	Component::Component(Object& owner) :
		object{ owner }
	{
		impl::DataManager::Register(this);
	}

	Component::~Component()
	{
		impl::DataManager::Unregister(this);
	}
}