#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"


namespace leopph::impl
{
	RenderComponent::RenderComponent(leopph::Entity& owner) :
		Component{owner}
	{
		DataManager::RegisterRenderComponent(this);
	}

	RenderComponent::~RenderComponent()
	{
		DataManager::UnregisterRenderComponent(this);
	}
}