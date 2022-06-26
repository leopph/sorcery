#include "RenderComponent.hpp"

#include "DataManager.hpp"


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

		if (m_Renderable && InUse())
		{
			dataManager.UnregisterActiveRenderComponent(m_Renderable, this);
		}

		Component::Owner(entity);

		if (m_Renderable && InUse())
		{
			dataManager.RegisterActiveRenderComponent(m_Renderable, this);
		}
	}


	auto RenderComponent::Active(bool const active) -> void
	{
		auto& dataManager = DataManager::Instance();

		if (m_Renderable && InUse())
		{
			dataManager.UnregisterActiveRenderComponent(m_Renderable, this);
		}

		Component::Active(active);

		if (m_Renderable && InUse())
		{
			dataManager.RegisterActiveRenderComponent(m_Renderable, this);
		}
	}


	RenderComponent::RenderComponent(RenderComponent const& other) :
		m_CastsShadow{other.m_CastsShadow},
		m_Instanced{other.m_Instanced},
		m_Renderable{other.m_Renderable}
	{
		if (m_Renderable && InUse())
		{
			DataManager::Instance().RegisterActiveRenderComponent(m_Renderable, this);
		}
	}


	auto RenderComponent::operator=(RenderComponent const& other) -> RenderComponent&
	{
		if (this == &other)
		{
			return *this;
		}

		auto& dataManager = DataManager::Instance();

		if (m_Renderable && InUse())
		{
			dataManager.UnregisterActiveRenderComponent(m_Renderable, this);
		}

		m_CastsShadow = other.m_CastsShadow;
		m_Instanced = other.m_Instanced;
		m_Renderable = other.m_Renderable;

		if (m_Renderable && InUse())
		{
			dataManager.RegisterActiveRenderComponent(m_Renderable, this);
		}

		return *this;
	}


	auto RenderComponent::SwapRenderable(MeshGroup meshGroup) -> void
	{
		auto& dataManager = DataManager::Instance();

		if (m_Renderable && InUse())
		{
			dataManager.UnregisterActiveRenderComponent(m_Renderable, this);
		}

		m_Renderable = std::make_shared<GlMeshGroup>(std::move(meshGroup));

		if (m_Renderable && InUse())
		{
			dataManager.RegisterActiveRenderComponent(m_Renderable, this);
		}
	}


	RenderComponent::~RenderComponent() noexcept
	{
		if (m_Renderable)
		{
			DataManager::Instance().UnregisterActiveRenderComponent(m_Renderable, this);
		}
	}
}
