#include "Component.hpp"


namespace leopph
{
	Entity* Component::get_owner() const
	{
		return mOwner;
	}



	Component::~Component() = default;
}
