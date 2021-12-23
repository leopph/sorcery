#pragma once

#include "../Event.hpp"

#include <type_traits>


namespace leopph::internal
{
	class EventReceiverBase
	{
		public:
			/* The parameter type of the handler. */
			using EventParamType = const std::decay_t<Event>&;

			LEOPPHAPI EventReceiverBase() = default;
			LEOPPHAPI EventReceiverBase(const EventReceiverBase& other) = default;
			LEOPPHAPI EventReceiverBase(EventReceiverBase&& other) = default;
			LEOPPHAPI auto operator=(const EventReceiverBase& other) -> EventReceiverBase& = default;
			LEOPPHAPI auto operator=(EventReceiverBase&& other) -> EventReceiverBase& = default;
			LEOPPHAPI virtual ~EventReceiverBase() = default;

			/* Invokes handler. */
			LEOPPHAPI virtual auto Handle(EventParamType event) const -> void = 0;

			/* Handler function. */
			LEOPPHAPI virtual auto operator==(const EventReceiverBase& other) const -> bool;
	};
}
