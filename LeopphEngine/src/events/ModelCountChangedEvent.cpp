#include "ModelCountChangedEvent.hpp"

#include "../data/DataManager.hpp"


namespace leopph::impl
{
	ModelCountChangedEvent::ModelCountChangedEvent(const ModelResource* modelResource) :
		Count{DataManager::ModelComponents(modelResource).size()}, Model{modelResource}
	{
	}
}