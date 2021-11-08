#pragma once

#include "EventManager.hpp"
#include "EventReceiverBase.hpp"
#include "../Event.hpp"
#include "../../api/LeopphApi.hpp"

#include <concepts>


namespace leopph
{
	/* The EventReceiver abstract base class provides an interface
	 * for receiving events. By subclassing this and overriding the
	 * OnEventReceived function, you can implement your own custom
	 * event handling logic.
	 * Instances of EventReceiver are automatically registered at
	 * the EventManager and they're also unregistered during destruction. */
	template<std::derived_from<Event> EventType>
	class EventReceiver : public impl::EventReceiverBase
	{
	public:
		/* The parameter type used in the signature of handler function. */
		using EventParamType = const std::decay_t<EventType>&;


		EventReceiver()
		{
			EventManager::Instance().RegisterFor<EventType>(*this);
		}


		LEOPPHAPI EventReceiver(const EventReceiver& other) = default;
		LEOPPHAPI EventReceiver(EventReceiver&& other) = default;
		LEOPPHAPI EventReceiver& operator=(const EventReceiver& other) = default;
		LEOPPHAPI EventReceiver& operator=(EventReceiver&& other) = default;


		~EventReceiver() override
		{
			EventManager::Instance().UnregisterFrom<EventType>(*this);
		}


		/* Used internally to invoke the receiver. Do not override or call this explicitly. */
		void Handle(EventReceiverBase::EventParamType event) const override
		{
			const_cast<EventReceiver*>(this)->OnEventReceived(static_cast<EventParamType>(event));
		}


	private:
		/* The handler function that is invoked when an event is sent.
		 * Implement this to provide your event handling logic. */
		LEOPPHAPI virtual void OnEventReceived(EventParamType event) = 0;
	};
}