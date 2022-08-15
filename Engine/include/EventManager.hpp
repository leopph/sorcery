#pragma once

#include "EventReceiverBase.hpp"
#include "LeopphApi.hpp"

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


	class EventManager
	{
		public:
			[[nodiscard]] LEOPPHAPI static EventManager& get_instance();

			template<std::derived_from<Event> EventType, class... Args>
			EventManager& send(Args&&... args);

			void register_receiver(std::type_index const& typeIndex, internal::EventReceiverBase* receiver);
			void unregister_receiver(std::type_index const& typeIndex, internal::EventReceiverBase const* receiver);


		private:
			EventManager() = default;

		public:
			EventManager(EventManager const& other) = delete;
			EventManager(EventManager&& other) = delete;

			void operator=(EventManager const& other) = delete;
			void operator=(EventManager&& other) = delete;

		private:
			~EventManager() = default;

			std::unordered_map<std::type_index, std::vector<internal::EventReceiverBase*>> mReceivers;
	};



	template<std::derived_from<Event> EventType, class... Args>
	EventManager& EventManager::send(Args&&... args)
	{
		if (auto const it = mReceivers.find(typeid(EventType)); it != std::end(mReceivers))
		{
			EventType event{std::forward<Args>(args)...};

			for (auto* const receiver : it->second)
			{
				receiver->internal_handle_event(event);
			}
		}

		return *this;
	}
}
