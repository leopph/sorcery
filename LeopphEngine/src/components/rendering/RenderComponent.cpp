#include "RenderComponent.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/GlMeshGroup.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"


namespace leopph::internal
{
	RenderComponent::RenderComponent(leopph::Entity* const entity, std::shared_ptr<const MeshDataGroup> meshDataGroup) :
		Component{entity}
	{
		m_Renderable = std::make_unique<GlMeshGroup>(DataManager::Instance().CreateOrGetMeshGroup(std::move(meshDataGroup)));
		DataManager::Instance().RegisterInstanceForMeshGroup(*m_Renderable, this);
	}

	RenderComponent::~RenderComponent() noexcept
	{
		DataManager::Instance().UnregisterInstanceFromMeshGroup(*m_Renderable, this);
	}

	auto RenderComponent::CastsShadow() const -> bool
	{
		return m_CastsShadow;
	}

	auto RenderComponent::CastsShadow(const bool value) -> void
	{
		m_CastsShadow = value;
	}

	auto RenderComponent::Instanced() const -> bool
	{
		return m_Instanced;
	}

	auto RenderComponent::Instanced(const bool value) -> void
	{
		m_Instanced = value;
	}
}
