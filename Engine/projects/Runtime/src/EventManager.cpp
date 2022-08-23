#include "EventManager.hpp"


namespace leopph
{
	EventManager& EventManager::get_instance()
	{
		static EventManager instance;
		return instance;
	}



	void EventManager::register_receiver(std::type_index const& typeIndex, internal::EventReceiverBase* receiver)
	{
		mReceivers[typeIndex].push_back(receiver);
	}



	void EventManager::unregister_receiver(std::type_index const& typeIndex, internal::EventReceiverBase const* receiver)
	{
		std::erase(mReceivers[typeIndex], receiver);
	}
}
