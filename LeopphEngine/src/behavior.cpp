#include "behavior.h"

namespace leopph
{
	Behavior::Behavior(Object& object) :
		m_Object{ object }
	{}

	Object& Behavior::OwningObject()
	{
		return m_Object;
	}
}