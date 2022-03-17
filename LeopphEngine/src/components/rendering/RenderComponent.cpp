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

		if (IsAttached())
		{
			auto& dataManager{DataManager::Instance()};
			dataManager.UnregisterInstanceFromMeshGroup(m_Renderable, this, false);
			dataManager.RegisterInstanceForMeshGroup(m_Renderable, this, true);
		}
	}


	auto RenderComponent::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		Component::Deactivate();

		if (IsAttached())
		{
			auto& dataManager{DataManager::Instance()};
			dataManager.UnregisterInstanceFromMeshGroup(m_Renderable, this, true);
			dataManager.RegisterInstanceForMeshGroup(m_Renderable, this, false);
		}
	}


	auto RenderComponent::Attach(leopph::Entity* entity) -> void
	{
		if (IsAttached())
		{
			return;
		}

		Component::Attach(entity);

		if (IsActive())
		{
			auto& dataManager{DataManager::Instance()};
			dataManager.UnregisterInstanceFromMeshGroup(m_Renderable, this, false);
			dataManager.RegisterInstanceForMeshGroup(m_Renderable, this, true);
		}
	}


	auto RenderComponent::Detach() -> void
	{
		if (!IsAttached())
		{
			return;
		}

		Component::Detach();

		if (IsActive())
		{
			auto& dataManager{DataManager::Instance()};
			dataManager.UnregisterInstanceFromMeshGroup(m_Renderable, this, true);
			dataManager.RegisterInstanceForMeshGroup(m_Renderable, this, false);
		}
	}


	RenderComponent::RenderComponent(std::shared_ptr<const MeshDataGroup> meshDataGroup) :
		m_Renderable{DataManager::Instance().CreateOrGetMeshGroup(std::move(meshDataGroup))}
	{
		DataManager::Instance().RegisterInstanceForMeshGroup(m_Renderable, this, false);
	}


	RenderComponent::~RenderComponent() noexcept
	{
		if (auto& dataManager{DataManager::Instance()}; dataManager.MeshGroupInstanceCount(m_Renderable) == 1ull)
		{
			dataManager.DestroyMeshGroup(m_Renderable);
		}
		else
		{
			dataManager.UnregisterInstanceFromMeshGroup(m_Renderable, this, IsAttached() && IsActive());
		}
	}
}
