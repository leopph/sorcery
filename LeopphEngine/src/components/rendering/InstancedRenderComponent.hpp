#pragma once

#include "RenderComponent.hpp"


namespace leopph::impl
{
	class MeshDataGroup;
	class GlMeshGroup;

	class InstancedRenderComponent : public RenderComponent
	{
	protected:
		InstancedRenderComponent(leopph::Entity* entity, MeshDataGroup& modelData);
		
		~InstancedRenderComponent() override;

		GlMeshGroup& m_Impl;
	};
};