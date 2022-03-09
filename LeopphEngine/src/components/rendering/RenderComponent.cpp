#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"


namespace leopph::internal
{
	auto RenderComponent::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		Component::Activate();
		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterInactiveInstanceFromMeshGroup(m_Renderable, this);
		dataManager.RegisterActiveInstanceForMeshGroup(m_Renderable, this);
	}


	auto RenderComponent::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		Component::Deactivate();
		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterActiveInstanceFromMeshGroup(m_Renderable, this);
		dataManager.RegisterInactiveInstanceForMeshGroup(m_Renderable, this);
	}


	RenderComponent::RenderComponent(std::shared_ptr<const MeshDataGroup> meshDataGroup) :
		m_Renderable{DataManager::Instance().CreateOrGetMeshGroup(std::move(meshDataGroup))}
	{
		DataManager::Instance().RegisterActiveInstanceForMeshGroup(m_Renderable, this);
	}


	RenderComponent::~RenderComponent() noexcept
	{
		if (DataManager::Instance().MeshGroupInstanceCount(m_Renderable) == 1ull)
		{
			DataManager::Instance().DestroyMeshGroup(m_Renderable);
		}
		else
		{
			if (IsActive())
			{
				DataManager::Instance().UnregisterActiveInstanceFromMeshGroup(m_Renderable, this);
			}
			else
			{
				DataManager::Instance().UnregisterInactiveInstanceFromMeshGroup(m_Renderable, this);
			}
		}
	}
}
