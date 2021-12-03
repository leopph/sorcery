#pragma once

#include "RenderComponent.hpp"


namespace leopph::impl
{
	class ModelData;
	class NonInstancedRenderable;

	class NonInstancedRenderComponent : public RenderComponent
	{
	protected:
		NonInstancedRenderComponent(leopph::Entity& owner, ModelData& modelData);

		~NonInstancedRenderComponent() override;

		NonInstancedRenderable& m_Impl;
	};
}