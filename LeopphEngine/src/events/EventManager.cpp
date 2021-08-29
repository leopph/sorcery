#include "EventManager.hpp"


namespace leopph
{
	EventManager& EventManager::Instance()
	{
		static EventManager instance;
		return instance;
	}
}