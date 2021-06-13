#include "component.h"

namespace leopph
{
	// constructor
	Component::Component(leopph::Object& object) :
		m_Object{ object }
	{}

	// destructor
	Component::~Component() = default;



	leopph::Object& Component::Object()
	{
		return const_cast<leopph::Object&>(const_cast<const Component*>(this)->Object());
	}

	const leopph::Object& Component::Object() const
	{
		return m_Object;
	}
}