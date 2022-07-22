#include "EventManager.hpp"


namespace leopph
{
	EventManager& EventManager::Instance()
	{
		static EventManager instance;
		return instance;
	}


	void EventManager::InternalRegister(std::type_index const& typeIndex, internal::EventReceiverBase const* receiver)
	{
		m_Handlers[typeIndex].push_back(receiver);
	}


	void EventManager::InternalUregister(std::type_index const& typeIndex, internal::EventReceiverBase const* receiver)
	{
		auto& handlers{m_Handlers[typeIndex]};
		for (auto it{handlers.begin()}; it != handlers.end(); ++it)
		{
			if (*it == receiver)
			{
				handlers.erase(it);
				break;
			}
		}
	}
}
