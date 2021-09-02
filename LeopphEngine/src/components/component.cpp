#include "Component.hpp"

#include "../data/DataManager.hpp"

namespace leopph
{
	Component::Component(Object& owner) :
		object{ owner }
	{
		impl::DataManager::RegisterComponent(this);
	}

	Component::~Component()
	{
		impl::DataManager::UnregisterComponent(this);
	}
}