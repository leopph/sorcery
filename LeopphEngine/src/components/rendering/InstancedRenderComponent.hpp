#pragma once

#include "RenderComponent.hpp"


namespace leopph::impl
{
	class MeshDataCollection;
	class GlMeshCollection;

	class InstancedRenderComponent : public RenderComponent
	{
	protected:
		InstancedRenderComponent(leopph::Entity* entity, MeshDataCollection& modelData);
		
		~InstancedRenderComponent() override;

		GlMeshCollection& m_Impl;
	};
};