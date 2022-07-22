#pragma once

#include "EventReceiverBase.hpp"
#include "LeopphApi.hpp"

#include <algorithm>
#include <concepts>
#include <typeindex>
#include <unordered_map>
#include <utility>
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
			LEOPPHAPI static EventManager& Instance();

			// Send the specified event to all registered receivers.
			// This creates a new instance of the specified event by forwarding the passed arguments.
			// The function then calls all registered receivers with this event event instance.
			// The Event object's lifetime is the function call.
			template<std::derived_from<Event> EventType, class... Args>
			auto& Send(Args&&... args);

			// Internally used.
			// CUrrently eventreceivers call this on consruction.
			template<std::derived_from<Event> EventType>
			auto RegisterFor(EventReceiver<EventType> const& receiver);

			// Internally used.
			// Currently EventReceivers call this on destruction.
			template<std::derived_from<Event> EventType>
			auto UnregisterFrom(internal::EventReceiverBase const& handler);

			EventManager(EventManager const& other) = delete;
			void operator=(EventManager const& other) = delete;

			EventManager(EventManager&& other) = delete;
			void operator=(EventManager&& other) = delete;

		private:
			EventManager() = default;
			constexpr ~EventManager() = default;

			// To hide DataManager from interface. Not exported, cannot be explicitly called from outside the library.
			void InternalRegister(std::type_index const& typeIndex, internal::EventReceiverBase const* receiver);
			// To hide DataManager from interface. Not exported, cannot be explicitly called from outside the library.
			void InternalUregister(std::type_index const& typeIndex, internal::EventReceiverBase const* receiver);

			std::unordered_map<std::type_index, std::vector<internal::EventReceiverBase const*>> m_Handlers;
	};


	template<std::derived_from<Event> EventType, class... Args>
	auto& EventManager::Send(Args&&... args)
	{
		if (auto const it{m_Handlers.find(typeid(EventType))};
			it != m_Handlers.end())
		{
			EventType event{std::forward<Args>(args)...};
			std::for_each(it->second.begin(), it->second.end(), [&event](auto const& handler)
			{
				handler->Handle(event);
			});
		}
		return *this;
	}


	template<std::derived_from<Event> EventType>
	auto EventManager::RegisterFor(EventReceiver<EventType> const& receiver)
	{
		InternalRegister(typeid(EventType), &receiver);
	}


	template<std::derived_from<Event> EventType>
	auto EventManager::UnregisterFrom(internal::EventReceiverBase const& handler)
	{
		InternalUregister(typeid(EventType), &handler);
	}
}
