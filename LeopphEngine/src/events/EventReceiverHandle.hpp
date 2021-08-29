#pragma once

#include "../api/leopphapi.h"
#include "Event.hpp"
#include "EventReceiver.hpp"

#include <concepts>
#include <cstddef>
#include <functional>


namespace leopph
{
	/* The EventReceiverHandle class provides a way to wrap individual functions
	 * into EventReceivers. Registering a function at the EventManager returns an instance
	 * of this class that acts as a reference particular internally stored receiver.
	 * Make sure to store store handles as the only way to unregister listeners registered
	 * this way is by passing the handle to the EventManager. */
	template<std::derived_from<Event> EventType>
	class EventReceiverHandle final : public EventReceiver<EventType>
	{
	public:
		/* The signature of the handler function. */
		using EventCallbackType = void(typename EventReceiver<EventType>::EventParamType);


		explicit EventReceiverHandle(std::function<EventCallbackType> callback) :
			m_Callback{ std::move(callback) }, m_Id{ s_NextId++ }
		{}


		LEOPPHAPI EventReceiverHandle(const EventReceiverHandle& other) = default;
		LEOPPHAPI EventReceiverHandle(EventReceiverHandle&& other) = default;
		LEOPPHAPI EventReceiverHandle& operator=(const EventReceiverHandle& other) = default;
		LEOPPHAPI EventReceiverHandle& operator=(EventReceiverHandle&& other) = default;
		LEOPPHAPI ~EventReceiverHandle() override = default;


		/* Handlers are equal if they refer to the same internally stored receiver.
		 * Duplicated handlers are equal, but handler referring to identical but
		 * separately registered receivers are not. */
		bool operator==(const impl::EventReceiverBase& other) const override
		{
			const auto otherPtr{ dynamic_cast<const EventReceiverHandle*>(&other) };
			return otherPtr != nullptr && m_Id == otherPtr->m_Id;
		}


	private:
		void OnEventReceived(typename EventReceiver<EventType>::EventParamType event) override
		{
			m_Callback(event);
		}

		std::function<EventCallbackType> m_Callback;
		std::size_t m_Id;

		static std::size_t s_NextId;
	};


	template<std::derived_from<Event> EventType>
	std::size_t EventReceiverHandle<EventType>::s_NextId{};
}