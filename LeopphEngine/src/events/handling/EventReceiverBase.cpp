#include "EventReceiverBase.hpp"


namespace leopph::internal
{
	bool EventReceiverBase::operator==(const EventReceiverBase& other) const
	{
		return this == &other;
	}
}