#include "Component.hpp"

#include "../data/DataManager.hpp"

namespace leopph
{
	Component::Component(leopph::Entity& owner) :
		Entity{ owner }
	{
		impl::DataManager::Register(this);
	}

	Component::~Component()
	{
		impl::DataManager::Unregister(this);
	}
}