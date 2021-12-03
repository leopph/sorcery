#include "NonInstancedRenderComponent.hpp"

#include "../../data/DataManager.hpp"


namespace leopph::impl
{
	NonInstancedRenderComponent::NonInstancedRenderComponent(leopph::Entity& owner, ModelData& modelData) :
		RenderComponent{owner}, m_Impl{DataManager::CreateNonInstancedRenderable(modelData, this)}
	{}

	NonInstancedRenderComponent::~NonInstancedRenderComponent()
	{
		DataManager::DestroyNonInstancedRenderable(m_Impl);
	}
}