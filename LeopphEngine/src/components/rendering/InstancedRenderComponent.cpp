#include "InstancedRenderComponent.hpp"

#include "../../data/DataManager.hpp"


namespace leopph::impl
{
	InstancedRenderComponent::InstancedRenderComponent(leopph::Entity* const entity, ModelData& modelData) :
		RenderComponent{entity},
		m_Impl{DataManager::CreateOrGetInstancedRenderable(modelData)}
	{
		DataManager::RegisterInstancedRenderComponent(m_Impl, this);
	}

	InstancedRenderComponent::~InstancedRenderComponent()
	{
		if (impl::DataManager::InstancedRenderables().at(m_Impl).size() == 1ull)
		{
			impl::DataManager::DestroyInstancedRenderable(m_Impl);
		}
		else
		{
			impl::DataManager::UnregisterInstancedRenderComponent(m_Impl, this);
		}
	}
}
