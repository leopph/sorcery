#include "rendering/RenderObject.hpp"

#include "Entity.hpp"


namespace leopph::internal
{
	void RenderObject::RegisterRenderComponent(RenderComponent* const renderComponent)
	{
		m_Components.push_back(renderComponent);
	}


	void RenderObject::UnregisterRenderComponent(RenderComponent* renderComponent)
	{
		std::erase(m_Components, renderComponent);
	}


	u64 RenderObject::NumRenderComponents() const
	{
		return m_Components.size();
	}


	std::span<RenderObject::RenderInstanceData const> RenderObject::ExtractRenderInstanceData()
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


	std::span<RenderObject::RenderInstanceData const> RenderObject::GetRenderInstanceData() const
	{
		return m_RenderInstancesCache;
	}
}
