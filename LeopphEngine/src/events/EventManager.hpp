#pragma once

#include "EventReceiverBase.hpp"
#include "../api/leopphapi.h"

#include <algorithm>
#include <concepts>
#include <typeindex>
#include <unordered_map>
#include <vector>



namespace leopph
{
	template<std::derived_from<Event> EventType>
	class EventReceiver;

	template<std::derived_from<Event> EventType>
	class EventReceiverHandle;


	/* The EventManager singleton is the center of the Event System.
	 * It registers receivers for events and manages event dispatch. */
	class EventManager
	{
		public:
			EventManager(const EventManager&) = delete;
			EventManager(EventManager&&) = delete;
			void operator=(const EventManager&) = delete;
			void operator=(EventManager&&) = delete;


			LEOPPHAPI static EventManager& Instance();


			/* Send the specified event to all registered receivers.
			 * This creates a new instance of the specified event with
			 * the given arguments and passes it to all of registed receivers.
			 * The Event object is not guaranteed to live past this function call. */
			template<std::derived_from<Event> EventType, class... Args>
			EventManager& Send(Args&&... args)
			{
				if (const auto it{m_Handlers.find(typeid(EventType))};
					it != m_Handlers.end())
				{
					EventType event{args...};
					std::for_each(it->second.begin(), it->second.end(), [&event](auto& handler)
					{
						handler->Handle(event);
					});
				}
				return *this;
			}


			/* Register as a receiver.
			 * This is used internally by instances of
			 * EventReceiver, and should not be called explicitly. */
			template<std::derived_from<Event> EventType>
			void RegisterFor(const EventReceiver<EventType>& receiver) // TODO create internal implementation with no dllexport
			{
				m_Handlers[typeid(EventType)].push_back(const_cast<EventReceiver<EventType>*>(&receiver));
			}


			/* Unregister the given receiver.
			 * For EventReceivers this is automatically called and requires
			 * no user input at all.
			 * For Handles, this invalidates all instances referring to the
			 * unregistered receiver. */
			template<std::derived_from<Event> EventType>
			EventManager& UnregisterFrom(const impl::EventReceiverBase& handler) // TODO create internal implementation with no dllexport
			{
				auto& handlers{m_Handlers[typeid(EventType)]};
				for (auto it{handlers.begin()}; it != handlers.end(); ++it)
				{
					if (*it == &handler)
					{
						handlers.erase(it);
						break;
					}
				}
				return *this;
			}


		private:
			EventManager() = default;
			~EventManager() = default;


			std::unordered_map<std::type_index, std::vector<impl::EventReceiverBase*>> m_Handlers;
	};
}
