#include "Component.hpp"


namespace leopph
{
	Component::Component(leopph::Entity& owner) :
		Entity{owner}
	{}

	Component::~Component() = default;
}
