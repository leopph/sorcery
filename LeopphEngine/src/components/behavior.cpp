#include "Behavior.hpp"

#include "../instances/InstanceHolder.hpp"

namespace leopph
{
	Behavior::Behavior(Object& owner) :
		Component{ owner }
	{
		impl::InstanceHolder::RegisterBehavior(this);
	}

	Behavior::~Behavior()
	{
		impl::InstanceHolder::UnregisterBehavior(this);
	}
}