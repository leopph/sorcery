#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/MeshGroup.hpp"


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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable.get(), this, false);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable.get(), this, true);
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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable.get(), this, true);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable.get(), this, false);
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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable.get(), this, false);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable.get(), this, true);
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
			dataManager.UnregisterInstanceFromGlMeshGroup(m_Renderable.get(), this, true);
			dataManager.RegisterInstanceForGlMeshGroup(m_Renderable.get(), this, false);
		}
	}


	RenderComponent::RenderComponent(std::shared_ptr<const MeshGroup> meshDataGroup) :
		m_Renderable{DataManager::Instance().FindGlMeshGroup(meshDataGroup.get())}
	{
		if (!m_Renderable)
		{
			m_Renderable = std::make_shared<GlMeshGroup>(std::move(meshDataGroup));
		}

		DataManager::Instance().RegisterInstanceForGlMeshGroup(m_Renderable.get(), this, false);
	}


	RenderComponent::~RenderComponent() noexcept
	{
		DataManager::Instance().UnregisterInstanceFromGlMeshGroup(m_Renderable.get(), this, IsAttached() && IsActive());
	}
}
