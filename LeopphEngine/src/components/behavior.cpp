#include "behavior.h"
#include "../instances/instanceholder.h"

namespace leopph
{
	Behavior::Behavior()
	{
		impl::InstanceHolder::AddBehavior(this);
	}

	Behavior::~Behavior()
	{
		impl::InstanceHolder::RemoveBehavior(this);
	}
}