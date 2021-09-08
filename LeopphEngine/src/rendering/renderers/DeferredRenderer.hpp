#pragma once

#include "../../math/Matrix.hpp"
#include "../CascadedShadowMap.hpp"
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
		void RenderDirectionalLights();
		void RenderSkybox() const;

		GeometryBuffer m_GBuffer;
		CascadedShadowMap m_DirShadowMap;
		ScreenTexture m_ScreenTexture;

		Shader m_GPassObjectShader;
		Shader m_LightPassShader;
		Shader m_SkyboxShader;
		Shader m_DirShadowShader;
		Shader m_DirLightShader;
		Shader m_TextureShader;

		std::vector<Matrix4> m_CurrentFrameDirLightMatrices;
	};
}