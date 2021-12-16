#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/GlMeshGroup.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"


namespace leopph::impl
{
	RenderComponent::RenderComponent(leopph::Entity* const entity, const MeshDataGroup& meshDataGroup) :
		Component{entity}
	{
		m_Renderable = std::make_unique<GlMeshGroup>(DataManager::CreateOrGetMeshGroup(meshDataGroup));
		DataManager::RegisterInstanceForMeshGroup(*m_Renderable, this);
	}

	RenderComponent::~RenderComponent() noexcept
	{
		DataManager::UnregisterInstanceFromMeshGroup(*m_Renderable, this);
	}

	bool RenderComponent::CastsShadow() const
	{
		return m_CastsShadow;
	}

	void RenderComponent::CastsShadow(const bool value)
	{
		m_CastsShadow = value;
	}

	bool RenderComponent::Instanced() const
	{
		return m_Instanced;
	}

	void RenderComponent::Instanced(const bool value)
	{
		m_Instanced = value;
	}
}
