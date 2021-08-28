#include "Component.hpp"

#include "../instances/DataManager.hpp"

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