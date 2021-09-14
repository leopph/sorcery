#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../ScreenTexture.hpp"
#include "../shaders/DeferredAmbLightShader.hpp"
#include "../shaders/DeferredDirLightShader.hpp"
#include "../shaders/DeferredGeometryShader.hpp"
#include "../shaders/DirShadowMapShader.hpp"
#include "../shaders/SkyboxShader.hpp"
#include "../shaders/TextureShader.hpp"



namespace leopph::impl
{
	class DeferredRenderer final : public Renderer
	{
		public:
			DeferredRenderer();

			DeferredRenderer(const DeferredRenderer& other) = default;
			DeferredRenderer(DeferredRenderer&& other) = default;

			DeferredRenderer& operator=(const DeferredRenderer& other) = default;
			DeferredRenderer& operator=(DeferredRenderer&& other) = default;

			~DeferredRenderer() override = default;

			void Render() override;


		private:
			void RenderGeometry();
			void RenderLights();
			void RenderAmbientLight() const;
			void RenderDirectionalLights();
			void RenderSkybox() const;

			GeometryBuffer m_GBuffer;
			CascadedShadowMap m_DirShadowMap;
			ScreenTexture m_ScreenTexture;

			DeferredGeometryShader m_GPassObjectShader;
			//Shader m_LightPassShader;
			SkyboxShader m_SkyboxShader;
			DirShadowMapShader m_DirShadowShader;
			DeferredDirLightShader m_DirLightShader;
			TextureShader m_TextureShader;
			DeferredAmbLightShader m_AmbientShader;
	};
}
