#include "EventReceiverBase.hpp"


namespace leopph::impl
{
	bool EventReceiverBase::operator==(const EventReceiverBase& other) const
	{
		return this == &other;
	}
}