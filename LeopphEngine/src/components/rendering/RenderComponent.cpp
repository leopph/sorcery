#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"


namespace leopph::internal
{
	RenderComponent::RenderComponent(leopph::Entity* const entity, std::shared_ptr<const MeshDataGroup> meshDataGroup) :
		Component{entity},
		m_Renderable{DataManager::Instance().CreateOrGetMeshGroup(std::move(meshDataGroup))}
	{
		DataManager::Instance().RegisterInstanceForMeshGroup(m_Renderable, this);
	}


	RenderComponent::~RenderComponent() noexcept
	{
		if (DataManager::Instance().MeshGroupInstanceCount(m_Renderable) == 1ull)
		{
			DataManager::Instance().DestroyMeshGroup(m_Renderable);
		}
		else
		{
			DataManager::Instance().UnregisterInstanceFromMeshGroup(m_Renderable, this);
		}
	}
}
