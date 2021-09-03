#pragma once

#include "../GeometryBuffer.hpp"
#include "../Shader.hpp"
#include "Renderer.hpp"


namespace leopph::impl
{
	class DeferredRenderer : public Renderer
	{
	public:
		DeferredRenderer();

		DeferredRenderer(const DeferredRenderer& other) = default;
		DeferredRenderer(DeferredRenderer&& other) = default;

		DeferredRenderer& operator=(const DeferredRenderer& other) = default;
		DeferredRenderer& operator=(DeferredRenderer&& other) = default;

		~DeferredRenderer() = default;

		void Render() override;

	private:
		void DrawGeometry();
		void DrawLights();

		GeometryBuffer m_GBuffer;

		Shader m_GPassObjectShader;
		Shader m_LightPassShader;
	};
}