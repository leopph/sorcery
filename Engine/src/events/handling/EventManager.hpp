#pragma once

#include "EventReceiverBase.hpp"
#include "../../api/LeopphApi.hpp"

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
			LEOPPHAPI static auto Instance() -> EventManager&;

			// Send the specified event to all registered receivers.
			// This creates a new instance of the specified event by forwarding the passed arguments.
			// The function then calls all registered receivers with this event event instance.
			// The Event object's lifetime is the function call.
			template<std::derived_from<Event> EventType, class... Args>
			auto Send(Args&&... args) -> auto&;

			// Internally used.
			// CUrrently eventreceivers call this on consruction.
			template<std::derived_from<Event> EventType>
			auto RegisterFor(const EventReceiver<EventType>& receiver);

			// Internally used.
			// Currently EventReceivers call this on destruction.
			template<std::derived_from<Event> EventType>
			auto UnregisterFrom(const internal::EventReceiverBase& handler);

			EventManager(const EventManager& other) = delete;
			auto operator=(const EventManager& other) -> void = delete;

			EventManager(EventManager&& other) = delete;
			auto operator=(EventManager&& other) -> void = delete;

		private:
			EventManager() = default;
			constexpr ~EventManager() = default;

			// To hide DataManager from interface. Not exported, cannot be explicitly called from outside the library.
			auto InternalRegister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver) -> void;
			// To hide DataManager from interface. Not exported, cannot be explicitly called from outside the library.
			auto InternalUregister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver) -> void;

			std::unordered_map<std::type_index, std::vector<const internal::EventReceiverBase*>> m_Handlers;
	};


	template<std::derived_from<Event> EventType, class... Args>
	auto EventManager::Send(Args&&... args) -> auto&
	{
		if (const auto it{m_Handlers.find(typeid(EventType))};
			it != m_Handlers.end())
		{
			EventType event{std::forward<Args>(args)...};
			std::for_each(it->second.begin(), it->second.end(), [&event](const auto& handler)
			{
				handler->Handle(event);
			});
		}
		return *this;
	}


	template<std::derived_from<Event> EventType>
	auto EventManager::RegisterFor(const EventReceiver<EventType>& receiver)
	{
		InternalRegister(typeid(EventType), &receiver);
	}


	template<std::derived_from<Event> EventType>
	auto EventManager::UnregisterFrom(const internal::EventReceiverBase& handler)
	{
		InternalUregister(typeid(EventType), &handler);
	}
}
