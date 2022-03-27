#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"


namespace leopph::internal
{
	auto RenderComponent::CastsShadow() const noexcept -> bool
	{
		return m_CastsShadow;
	}


	auto RenderComponent::CastsShadow(bool const value) noexcept -> void
	{
		m_CastsShadow = value;
	}


	auto RenderComponent::Instanced() const noexcept -> bool
	{
		return m_Instanced;
	}


	auto RenderComponent::Instanced(bool const value) noexcept -> void
	{
		m_Instanced = value;
	}


	auto RenderComponent::Owner(Entity* entity) -> void
	{
		auto& dataManager = DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
		}

		Component::Owner(entity);

		if (InUse())
		{
			dataManager.RegisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
		}
	}


	auto RenderComponent::Active(bool const active) -> void
	{
		auto& dataManager = DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
		}

		Component::Active(active);

		if (InUse())
		{
			dataManager.RegisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
		}
	}


	auto RenderComponent::operator=(RenderComponent const& other) -> RenderComponent&
	{
		if (this == &other)
		{
			return *this;
		}

		auto& dataManager = DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
		}

		m_CastsShadow = other.m_CastsShadow;
		m_Instanced = other.m_Instanced;
		m_Renderable = other.m_Renderable;

		if (InUse())
		{
			dataManager.RegisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
		}

		return *this;
	}


	RenderComponent::RenderComponent(std::shared_ptr<MeshGroup const>&& meshGroup) :
		m_Renderable{GlMeshGroup::CreateOrGet(std::move(meshGroup))}
	{
		DataManager::Instance().RegisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
	}


	RenderComponent::~RenderComponent() noexcept
	{
		DataManager::Instance().UnregisterActiveRenderComponent(m_Renderable->MeshGroup()->Id, this);
	}
}
