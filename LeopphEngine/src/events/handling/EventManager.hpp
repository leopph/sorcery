#pragma once

#include "EventReceiverBase.hpp"
#include "../../api/LeopphApi.hpp"

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
					std::for_each(it->second.begin(), it->second.end(), [&event](const auto& handler)
					{
						handler->Handle(event);
					});
				}
				return *this;
			}


			/* Internally used. */
			template<std::derived_from<Event> EventType>
			void RegisterFor(const EventReceiver<EventType>& receiver)
			{
				InternalRegister(typeid(EventType), &receiver);
			}


			/* Internally used. */
			template<std::derived_from<Event> EventType>
			void UnregisterFrom(const impl::EventReceiverBase& handler)
			{
				InternalUregister(typeid(EventType), &handler);
			}


		private:
			EventManager() = default;
			~EventManager() = default;

			void InternalRegister(const std::type_index& typeIndex, const impl::EventReceiverBase* receiver);
			void InternalUregister(const std::type_index& typeIndex, const impl::EventReceiverBase* receiver);


			std::unordered_map<std::type_index, std::vector<const impl::EventReceiverBase*>> m_Handlers;
	};
}
