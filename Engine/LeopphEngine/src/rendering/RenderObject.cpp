#include "rendering/RenderObject.hpp"

#include "Entity.hpp"


namespace leopph::internal
{
	auto RenderObject::RegisterRenderComponent(RenderComponent* const renderComponent) -> void
	{
		m_Components.push_back(renderComponent);
	}


	auto RenderObject::UnregisterRenderComponent(RenderComponent* renderComponent) -> void
	{
		std::erase(m_Components, renderComponent);
	}


	auto RenderObject::NumRenderComponents() const -> u64
	{
		return static_cast<u64>(m_Components.size());
	}


	auto RenderObject::ExtractRenderInstanceData() -> std::span<RenderInstanceData const>
	{
		m_RenderInstancesCache.clear();

		for (auto const* const comp : m_Components)
		{
			if (comp->InUse())
			{
				RenderInstanceData instanceData;
				auto const& [world, normal] = comp->Owner()->get_transform().get_matrices();
				instanceData.WorldTransform = world;
				instanceData.NormalTransform = normal;
				instanceData.CastsShadow = comp->CastsShadow();
				instanceData.IsInstanced = comp->Instanced();
				m_RenderInstancesCache.push_back(instanceData);
			}
		}

		return m_RenderInstancesCache;
	}


	auto RenderObject::GetRenderInstanceData() const -> std::span<RenderInstanceData const>
	{
		return m_RenderInstancesCache;
	}
}
