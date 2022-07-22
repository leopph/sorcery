#pragma once

#include "Event.hpp"
#include "EventReceiver.hpp"

#include <concepts>
#include <functional>
#include <memory>


namespace leopph
{
	// The EventReceiverHandle class provides a way to wrap individual functions into EventReceivers.
	template<std::derived_from<Event> EventType>
	class EventReceiverHandle final
	{
		public:
			// The signature of the handler function.
			using HandlerType = void(typename EventReceiver<EventType>::EventParamType);

			explicit EventReceiverHandle(std::function<HandlerType> callback);

		private:
			class InternalReceiver;
			// Pointer to the actual receiver object.
			std::shared_ptr<InternalReceiver> m_Receiver;
	};


	// Nested private class of EventReceiverHandle.
	// Used to make copying handlers around possible.
	// Encapsulates a callable that it calls on events.
	template<std::derived_from<Event> EventType>
	class EventReceiverHandle<EventType>::InternalReceiver final : public EventReceiver<EventType>
	{
		public:
			explicit InternalReceiver(std::function<HandlerType> callback);

			InternalReceiver(InternalReceiver const& other) = delete;
			InternalReceiver& operator=(InternalReceiver const& other) = delete;

			InternalReceiver(InternalReceiver&& other) noexcept = delete;
			InternalReceiver& operator=(InternalReceiver&& other) noexcept = delete;

			~InternalReceiver() override = default;

		private:
			void OnEventReceived(typename EventReceiver<EventType>::EventParamType event) override;

			std::function<HandlerType> m_Callback;
	};


	template<std::derived_from<Event> EventType>
	EventReceiverHandle<EventType>::EventReceiverHandle(std::function<HandlerType> callback) :
		m_Receiver{std::make_shared<InternalReceiver>(std::move(callback))}
	{}


	template<std::derived_from<Event> EventType>
	EventReceiverHandle<EventType>::InternalReceiver::InternalReceiver(std::function<HandlerType> callback) :
		m_Callback{std::move(callback)}
	{}


	template<std::derived_from<Event> EventType>
	void EventReceiverHandle<EventType>::InternalReceiver::OnEventReceived(typename EventReceiver<EventType>::EventParamType event)
	{
		m_Callback(event);
	}
}
