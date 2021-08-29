#pragma once

#include "Event.hpp"
#include "EventHandler.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>


namespace leopph
{
	class EventManager
	{
	public:
		static EventManager& Instance();


		template<std::derived_from<Event> EventType, class... Args>
		EventManager& Send(Args&&... args)
		{
			if (const auto it{ m_HandlersByEvents.find(typeid(EventType)) };
				it != m_HandlersByEvents.end())
			{
				EventType event{ args... };
				std::for_each(it->second.begin(), it->second.end(), [&event](auto& callback)
				{
					callback(event);
				});
			}
			return *this;
		}


		template<std::derived_from<Event> EventType>
		EventHandler& RegisterFor(const std::function<EventHandler::CallbackType>& func)
		{
			auto& handlers{ m_HandlersByEvents[typeid(EventType)] };
			handlers.emplace_back(func);
			return handlers.back();
		}


		template<std::derived_from<Event> EventType>
		EventManager& UnregisterFrom(const EventHandler& handler)
		{
			auto& handlers{ m_HandlersByEvents[typeid(EventType)] };
			for (auto it{ handlers.begin() }; it != handlers.end(); ++it)
			{
				if (*it == handler)
				{
					handlers.erase(it);
					break;
				}
			}
			return *this;
		}


	private:
		std::unordered_map<std::type_index, std::vector<EventHandler>> m_HandlersByEvents;

		EventManager() = default;
	};
}