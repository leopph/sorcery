#pragma once

#include "../GeometryBuffer.hpp"
#include "../ScreenTexture.hpp"
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
		void RenderGeometry();
		void RenderLights();
		void RenderSkybox() const;

		GeometryBuffer m_GBuffer;
		ScreenTexture m_ScreenTexture;

		Shader m_GPassObjectShader;
		Shader m_LightPassShader;
		Shader m_SkyboxShader;
	};
}