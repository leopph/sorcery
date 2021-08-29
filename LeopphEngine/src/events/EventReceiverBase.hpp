#pragma once

#include "Event.hpp"

#include <type_traits>


namespace leopph::impl
{
	class EventReceiverBase
	{
	public:
		/* The parameter type of the handler. */
		using EventParamType = const std::decay_t<Event>&;

		LEOPPHAPI EventReceiverBase() = default;
		LEOPPHAPI EventReceiverBase(const EventReceiverBase& other) = default;
		LEOPPHAPI EventReceiverBase(EventReceiverBase&& other) = default;
		LEOPPHAPI EventReceiverBase& operator=(const EventReceiverBase& other) = default;
		LEOPPHAPI EventReceiverBase& operator=(EventReceiverBase&& other) = default;
		LEOPPHAPI virtual ~EventReceiverBase() = default;

		/* Invokes handler. */
		LEOPPHAPI virtual void Handle(EventParamType event) const = 0;

		/* Handler function. */
		LEOPPHAPI virtual bool operator==(const EventReceiverBase& other) const;
	};
}