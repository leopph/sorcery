#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	// Base class for all EventReceivers.
	class EventReceiverBase
	{
		public:
			// Internal function called by EventManager to dispatch virtual calls to the appropriate handler.
			virtual auto Handle(const Event& event) const -> void = 0;

			constexpr virtual ~EventReceiverBase() = default;

		protected:
			EventReceiverBase() = default;

			EventReceiverBase(const EventReceiverBase& other) = default;
			auto operator=(const EventReceiverBase& other) -> EventReceiverBase& = default;

			EventReceiverBase(EventReceiverBase&& other) noexcept = default;
			auto operator=(EventReceiverBase&& other) noexcept -> EventReceiverBase& = default;
	};
}
