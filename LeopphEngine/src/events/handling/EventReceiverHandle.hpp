#pragma once

#include "EventReceiver.hpp"
#include "../Event.hpp"

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

			EventReceiverHandle(const EventReceiverHandle& other) = default;
			auto operator=(const EventReceiverHandle& other) -> EventReceiverHandle& = default;

			EventReceiverHandle(EventReceiverHandle&& other) noexcept;
			auto operator=(EventReceiverHandle&& other) noexcept -> EventReceiverHandle&;

			constexpr ~EventReceiverHandle() noexcept = default;

			// Handlers are equal if they refer to the same internally stored receiver.
			// Duplicated handlers are equal, but handler referring to identical but separately registered receivers are not.
			auto operator==(const EventReceiverHandle& other) const -> bool = default;

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

			InternalReceiver(const InternalReceiver& other) = delete;
			auto operator=(const InternalReceiver& other) -> InternalReceiver& = delete;

			InternalReceiver(InternalReceiver&& other) noexcept = delete;
			auto operator=(InternalReceiver&& other) noexcept -> InternalReceiver& = delete;

			~InternalReceiver() override = default;

		private:
			auto OnEventReceived(typename EventReceiver<EventType>::EventParamType event) -> void override;

			std::function<HandlerType> m_Callback;
	};


	template<std::derived_from<Event> EventType>
	EventReceiverHandle<EventType>::EventReceiverHandle(std::function<HandlerType> callback) :
		m_Receiver{std::make_shared<InternalReceiver>(std::move(callback))}
	{}


	template<std::derived_from<Event> EventType>
	EventReceiverHandle<EventType>::EventReceiverHandle(EventReceiverHandle&& other) noexcept :
		EventReceiverHandle{other}
	{}


	template<std::derived_from<Event> EventType>
	auto EventReceiverHandle<EventType>::operator=(EventReceiverHandle&& other) noexcept -> EventReceiverHandle&
	{
		*this = other;
		return *this;
	}


	template<std::derived_from<Event> EventType>
	EventReceiverHandle<EventType>::InternalReceiver::InternalReceiver(std::function<HandlerType> callback) :
		m_Callback{std::move(callback)}
	{}


	template<std::derived_from<Event> EventType>
	auto EventReceiverHandle<EventType>::InternalReceiver::OnEventReceived(typename EventReceiver<EventType>::EventParamType event) -> void
	{
		m_Callback(event);
	}
}
