#include "ModelInstanceCountEvent.hpp"

#include "../data/DataManager.hpp"


namespace leopph::impl
{
	ModelInstanceCountEvent::ModelInstanceCountEvent(const ModelResource* modelResource) :
		Count{DataManager::ModelComponents(modelResource).size()}, Model{modelResource}
	{
	}
}