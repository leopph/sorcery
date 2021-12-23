#include "EventReceiverBase.hpp"


namespace leopph::internal
{
	auto EventReceiverBase::operator==(const EventReceiverBase& other) const -> bool
	{
		return this == &other;
	}
}
