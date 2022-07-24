#include "RenderComponent.hpp"

#include "../InternalContext.hpp"
#include "../rendering/Renderer.hpp"
#include "../rendering/RenderObject.hpp"


namespace leopph::internal
{
	bool RenderComponent::CastsShadow() const noexcept
	{
		return m_CastsShadow;
	}



	void RenderComponent::CastsShadow(bool const value) noexcept
	{
		m_CastsShadow = value;
	}



	bool RenderComponent::Instanced() const noexcept
	{
		return m_Instanced;
	}



	void RenderComponent::Instanced(bool const value) noexcept
	{
		m_Instanced = value;
	}



	void RenderComponent::Init(MeshGroup const& meshGroup) noexcept
	{
		m_RenderObject = GetRenderer()->create_render_object(meshGroup);
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



	RenderComponent& RenderComponent::operator=(RenderComponent const& other) noexcept
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



	RenderComponent& RenderComponent::operator=(RenderComponent&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		m_RenderObject->UnregisterRenderComponent(this);

		// If we are the last component referring to the RenderObject, it is our job to delete it.
		if (m_RenderObject->NumRenderComponents() == 0)
		{
			GetRenderer()->delete_render_object(m_RenderObject);
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
			GetRenderer()->delete_render_object(m_RenderObject);
		}
	}
}
