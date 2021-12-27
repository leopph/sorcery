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

	// The EventManager singleton is the center of the Event System.
	// It registers receivers for events and manages event dispatch.
	class EventManager
	{
		public:
			LEOPPHAPI static auto Instance() -> EventManager&;

			// Send the specified event to all registered receivers.
			// This creates a new instance of the specified event with
			// the given arguments and passes it to all of registed receivers.
			// The Event object is not guaranteed to live past this function call.
			template<std::derived_from<Event> EventType, class... Args>
			auto Send(Args&&... args) -> EventManager&
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


			// Internally used.
			template<std::derived_from<Event> EventType>
			auto RegisterFor(const EventReceiver<EventType>& receiver) -> void
			{
				InternalRegister(typeid(EventType), &receiver);
			}


			// Internally used.
			template<std::derived_from<Event> EventType>
			auto UnregisterFrom(const internal::EventReceiverBase& handler) -> void
			{
				InternalUregister(typeid(EventType), &handler);
			}


			EventManager(const EventManager& other) = delete;
			auto operator=(const EventManager& other) -> void = delete;

			EventManager(EventManager&& other) = delete;
			auto operator=(EventManager&& other) -> void = delete;

		private:
			EventManager() = default;
			~EventManager() = default;

			auto InternalRegister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver) -> void;
			auto InternalUregister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver) -> void;

			std::unordered_map<std::type_index, std::vector<const internal::EventReceiverBase*>> m_Handlers;
	};
}
