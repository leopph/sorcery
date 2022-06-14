#include "EventManager.hpp"


namespace leopph
{
	auto EventManager::Instance() -> EventManager&
	{
		static EventManager instance;
		return instance;
	}

	auto EventManager::InternalRegister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver) -> void
	{
		m_Handlers[typeIndex].push_back(receiver);
	}

	auto EventManager::InternalUregister(const std::type_index& typeIndex, const internal::EventReceiverBase* receiver) -> void
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
