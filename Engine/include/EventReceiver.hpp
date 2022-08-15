#pragma once

#include "Event.hpp"
#include "EventManager.hpp"
#include "EventReceiverBase.hpp"

#include <concepts>


namespace leopph
{
	template<std::derived_from<Event> EventType>
	class EventReceiver : public internal::EventReceiverBase
	{
		public:
			void internal_handle_event(Event const& event) override;


		protected:
			EventReceiver();

			EventReceiver(EventReceiver const& other);
			EventReceiver(EventReceiver&& other) noexcept;

			EventReceiver& operator=(EventReceiver const& other) = default;
			EventReceiver& operator=(EventReceiver&& other) noexcept = default;

		public:
			~EventReceiver() override;

		private:
			virtual void on_event(EventType const& event) = 0;
	};



	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::EventReceiver()
	{
		EventManager::get_instance().register_receiver(typeid(EventType), this);
	}



	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::EventReceiver(EventReceiver const&) :
		EventReceiver{}
	{}



	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::EventReceiver(EventReceiver&& other) noexcept :
		EventReceiver{other}
	{}



	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::~EventReceiver()
	{
		EventManager::get_instance().unregister_receiver(typeid(EventType), this);
	}



	template<std::derived_from<Event> EventType>
	void EventReceiver<EventType>::internal_handle_event(Event const& event)
	{
		on_event(static_cast<EventType const&>(event));
	}
}
