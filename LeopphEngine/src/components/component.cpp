#include "Component.hpp"


namespace leopph
{
	Component::Component(leopph::Entity* const owner) :
		m_Entity{owner}
	{}

	Entity* Component::Entity() const
	{
		return m_Entity;
	}

}
