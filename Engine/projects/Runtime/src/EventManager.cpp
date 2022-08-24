#include "Event.hpp"


namespace leopph
{
	Event::~Event() = default;


	EventManager& EventManager::get_instance()
	{
		static EventManager instance;
		return instance;
	}



	void EventManager::register_receiver(std::type_index const& typeIndex, EventReceiverBase* receiver)
	{
		mReceivers[typeIndex].push_back(receiver);
	}



	void EventManager::unregister_receiver(std::type_index const& typeIndex, EventReceiverBase const* receiver)
	{
		std::erase(mReceivers[typeIndex], receiver);
	}
}
