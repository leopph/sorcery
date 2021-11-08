#pragma once

#include "EventReceiver.hpp"
#include "../Event.hpp"
#include "../../api/LeopphApi.hpp"

#include <concepts>
#include <functional>
#include <memory>



namespace leopph
{
	/* The EventReceiverHandle class provides a way to wrap individual functions
	 * into EventReceivers. Creating a handle registers the wrapper in the EventManager.
	 * The internal receivers live as long as there are handles to them. */
	template<std::derived_from<Event> EventType>
	class EventReceiverHandle final
	{
		public:
			/* The signature of the handler function. */
			using EventCallbackType = void(typename EventReceiver<EventType>::EventParamType);


		private:
			class InternalReceiver final : public EventReceiver<EventType>
			{
				public:
					explicit InternalReceiver(std::function<EventCallbackType> callback) :
						m_Callback{std::move(callback)}
					{}


				private:
					void OnEventReceived(typename EventReceiver<EventType>::EventParamType event) override
					{
						m_Callback(event);
					}


					std::function<EventCallbackType> m_Callback;
			};



		public:
			LEOPPHAPI explicit EventReceiverHandle(std::function<EventCallbackType> callback) :
				m_Receiver{std::make_shared<InternalReceiver>(std::move(callback))}
			{}


			/* Handlers are equal if they refer to the same internally stored receiver.
			 * Duplicated handlers are equal, but handler referring to identical but
			 * separately registered receivers are not. */
			LEOPPHAPI bool operator==(const EventReceiverHandle& other) const
			{
				return m_Receiver == other.m_Receiver;
			}


		private:
			std::shared_ptr<InternalReceiver> m_Receiver;
	};
}
