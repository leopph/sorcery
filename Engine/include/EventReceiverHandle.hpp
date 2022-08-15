#pragma once

#include "Event.hpp"
#include "EventReceiver.hpp"

#include <concepts>
#include <functional>
#include <utility>


namespace leopph
{
	template<std::derived_from<Event> EventType>
	class EventReceiverHandle : public EventReceiver<EventType>
	{
		public:
			explicit EventReceiverHandle(std::function<void(EventType const&)> callback);

		private:
			void on_event(EventType const& event) override;

			std::function<void(EventType const&)> mCallback;
	};



	template<std::derived_from<Event> EventType>
	EventReceiverHandle<EventType>::EventReceiverHandle(std::function<void(EventType const&)> callback) :
		mCallback{std::move(callback)}
	{}



	template<std::derived_from<Event> EventType>
	void EventReceiverHandle<EventType>::on_event(EventType const& event)
	{
		mCallback(event);
	}
}
