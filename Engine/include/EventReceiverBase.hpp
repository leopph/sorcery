#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	// Base class for all EventReceivers.
	class EventReceiverBase
	{
		public:
			// Internal function called by EventManager to dispatch virtual calls to the appropriate handler.
			virtual void Handle(Event const& event) const = 0;

			constexpr virtual ~EventReceiverBase() = default;

		protected:
			EventReceiverBase() = default;

			EventReceiverBase(EventReceiverBase const& other) = default;
			EventReceiverBase& operator=(EventReceiverBase const& other) = default;

			EventReceiverBase(EventReceiverBase&& other) noexcept = default;
			EventReceiverBase& operator=(EventReceiverBase&& other) noexcept = default;
	};
}
