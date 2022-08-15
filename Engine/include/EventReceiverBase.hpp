#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	class EventReceiverBase
	{
		public:
			virtual void internal_handle_event(Event const& event) = 0;


		protected:
			EventReceiverBase() = default;

			EventReceiverBase(EventReceiverBase const& other) = default;
			EventReceiverBase(EventReceiverBase&& other) noexcept = default;

			EventReceiverBase& operator=(EventReceiverBase const& other) = default;
			EventReceiverBase& operator=(EventReceiverBase&& other) noexcept = default;

		public:
			virtual ~EventReceiverBase() = default;
	};
}
