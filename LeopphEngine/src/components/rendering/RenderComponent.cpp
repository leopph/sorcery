#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"


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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable->MeshGroup()->Id, this, false);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable->MeshGroup()->Id, this, true);
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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable->MeshGroup()->Id, this, true);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable->MeshGroup()->Id, this, false);
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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable->MeshGroup()->Id, this, false);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable->MeshGroup()->Id, this, true);
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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable->MeshGroup()->Id, this, true);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable->MeshGroup()->Id, this, false);
		}
	}


	RenderComponent::RenderComponent(std::shared_ptr<MeshGroup const>&& meshGroup) :
		m_Renderable{GlMeshGroup::CreateOrGet(std::move(meshGroup))}
	{

		DataManager::Instance().RegisterInstanceForGlMeshGroup(m_Renderable->MeshGroup()->Id, this, false);
	}


	RenderComponent::~RenderComponent() noexcept
	{
		DataManager::Instance().UnregisterInstanceFromGlMeshGroup(m_Renderable->MeshGroup()->Id, this, IsAttached() && IsActive());
	}
}
