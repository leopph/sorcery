#include "RenderComponent.hpp"

#include "InternalContext.hpp"
#include "rendering/RenderObject.hpp"
#include "rendering/renderers/Renderer.hpp"


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


	auto RenderComponent::Init(MeshGroup const& meshGroup) noexcept -> void
	{
		m_RenderObject = GetRenderer()->CreateRenderObject(meshGroup);
		m_RenderObject->RegisterRenderComponent(this);
	}


	RenderComponent::RenderComponent(RenderComponent const& other) noexcept :
		Component{other},
		m_CastsShadow{other.m_CastsShadow},
		m_Instanced{other.m_Instanced},
		m_RenderObject{other.m_RenderObject}
	{
		m_RenderObject->RegisterRenderComponent(this);
	}


	auto RenderComponent::operator=(RenderComponent const& other) noexcept -> RenderComponent&
	{
		if (this == &other)
		{
			return *this;
		}

		m_RenderObject->UnregisterRenderComponent(this);

		m_CastsShadow = other.m_CastsShadow;
		m_Instanced = other.m_Instanced;
		m_RenderObject = other.m_RenderObject;

		m_RenderObject->RegisterRenderComponent(this);

		return *this;
	}


	RenderComponent::RenderComponent(RenderComponent&& other) noexcept :
		Component{other},
		m_CastsShadow{other.m_CastsShadow},
		m_Instanced{other.m_Instanced},
		m_RenderObject{other.m_RenderObject}
	{
		m_RenderObject->RegisterRenderComponent(this);
	}


	auto RenderComponent::operator=(RenderComponent&& other) noexcept -> RenderComponent&
	{
		if (this == &other)
		{
			return *this;
		}

		m_RenderObject->UnregisterRenderComponent(this);

		// If we are the last component referring to the RenderObject, it is our job to delete it.
		if (m_RenderObject->NumRenderComponents() == 0)
		{
			GetRenderer()->DeleteRenderObject(m_RenderObject);
		}

		m_CastsShadow = other.m_CastsShadow;
		m_Instanced = other.m_Instanced;
		m_RenderObject = other.m_RenderObject;

		m_RenderObject->RegisterRenderComponent(this);

		return *this;
	}


	RenderComponent::~RenderComponent()
	{
		m_RenderObject->UnregisterRenderComponent(this);

		// If we are the last component referring to the RenderObject, it is our job to delete it.
		if (m_RenderObject->NumRenderComponents() == 0)
		{
			GetRenderer()->DeleteRenderObject(m_RenderObject);
		}
	}
}
