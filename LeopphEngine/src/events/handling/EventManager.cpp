#include "EventManager.hpp"


namespace leopph
{
	EventManager& EventManager::Instance()
	{
		static EventManager instance;
		return instance;
	}


	void EventManager::InternalRegister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver)
	{
		m_Handlers[typeIndex].push_back(receiver);
	}


	void EventManager::InternalUregister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver)
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