#pragma once

#include "EventManager.hpp"
#include "EventReceiverBase.hpp"
#include "Event.hpp"

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
			using EventParamType = const EventType&;

			// Internally used function to dispatch calls to the handler.
			auto Handle(const Event& event) const -> void override;

			~EventReceiver() override;

		protected:
			EventReceiver();

			EventReceiver(const EventReceiver& other);
			auto operator=(const EventReceiver& other) -> EventReceiver& = default;

			EventReceiver(EventReceiver&& other) noexcept;
			auto operator=(EventReceiver&& other) noexcept -> EventReceiver& = default;

		private:
			// The handler function that is invoked when an event is sent.
			// Implement this to provide your event handling logic.
			virtual auto OnEventReceived(EventParamType event) -> void = 0;
	};


	template<std::derived_from<Event> SpecEventType>
	EventReceiver<SpecEventType>::EventReceiver()
	{
		EventManager::Instance().RegisterFor<EventType>(*this);
	}


	template<std::derived_from<Event> SpecEventType>
	EventReceiver<SpecEventType>::EventReceiver(const EventReceiver&) :
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
	auto EventReceiver<SpecEventType>::Handle(const Event& event) const -> void
	{
		const_cast<EventReceiver*>(this)->OnEventReceived(static_cast<EventParamType>(event));
	}
}
