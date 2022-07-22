#pragma once

#include "Event.hpp"
#include "EventManager.hpp"
#include "EventReceiverBase.hpp"

#include <concepts>
#include <type_traits>


namespace leopph
{
	// The EventReceiver abstract base class provides an interface for receiving events.
	// By subclassing it and overriding the OnEventReceived function, you can implement your own custom event handling logic.
	// Instances of EventReceiver are automatically registered at the EventManager and they're also unregistered on destruction.
	// Const, volatile, reference, pointer, and other modifiers do NOT take part in the identification of the subscribed-to event.
	// E.g. subscribing to const MyEvent& and volatile MyEvent* will be equivalent and the underlying event type will be MyEvent.
	template<std::derived_from<Event> SpecEventType>
	class EventReceiver : public internal::EventReceiverBase
	{
		public:
			// The event type the receiver is subscribed to.
			using EventType = std::remove_pointer_t<std::decay_t<std::remove_cvref_t<SpecEventType>>>;

			// The parameter type used in the signature of handler function.
			using EventParamType = EventType const&;

			// Internally used function to dispatch calls to the handler.
			void Handle(Event const& event) const override;

			~EventReceiver() override;

		protected:
			EventReceiver();

			EventReceiver(EventReceiver const& other);
			EventReceiver& operator=(EventReceiver const& other) = default;

			EventReceiver(EventReceiver&& other) noexcept;
			EventReceiver& operator=(EventReceiver&& other) noexcept = default;

		private:
			// The handler function that is invoked when an event is sent.
			// Implement this to provide your event handling logic.
			virtual void OnEventReceived(EventParamType event) = 0;
	};


	template<std::derived_from<Event> SpecEventType>
	EventReceiver<SpecEventType>::EventReceiver()
	{
		EventManager::Instance().RegisterFor<EventType>(*this);
	}


	template<std::derived_from<Event> SpecEventType>
	EventReceiver<SpecEventType>::EventReceiver(EventReceiver const&) :
		EventReceiver{}
	{}


	template<std::derived_from<Event> SpecEventType>
	EventReceiver<SpecEventType>::EventReceiver(EventReceiver&& other) noexcept :
		EventReceiver{other}
	{}


	template<std::derived_from<Event> SpecEventType>
	EventReceiver<SpecEventType>::~EventReceiver()
	{
		EventManager::Instance().UnregisterFrom<EventType>(*this);
	}


	template<std::derived_from<Event> SpecEventType>
	void EventReceiver<SpecEventType>::Handle(Event const& event) const
	{
		const_cast<EventReceiver*>(this)->OnEventReceived(static_cast<EventParamType>(event));
	}
}
