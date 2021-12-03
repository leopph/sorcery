#pragma once

#include "RenderComponent.hpp"


namespace leopph::impl
{
	class ModelData;
	class InstancedRenderable;

	class InstancedRenderComponent : public RenderComponent
	{
	protected:
		InstancedRenderComponent(leopph::Entity& owner, ModelData& modelData);
		
		~InstancedRenderComponent() override;

		InstancedRenderable& m_Impl;
	};
};