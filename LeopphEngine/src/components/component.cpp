#include "Component.hpp"

#include "../instances/InstanceHolder.hpp"

namespace leopph
{
	Component::Component(Object& owner) :
		object{ owner }
	{
		impl::InstanceHolder::RegisterComponent(this);
	}

	Component::~Component()
	{
		impl::InstanceHolder::UnregisterComponent(this);
	}
}