#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../ScreenTexture.hpp"
#include "../../math/Matrix.hpp"
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

			void Render() override;


		private:
			void RenderGeometry(const Matrix4& camViewMat,
								const Matrix4& camProjMat,
								const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats) const;

			void RenderAmbientLight() const;

			void RenderDirectionalLights(const Matrix4& camViewMat,
										 const Matrix4& camProjMat,
										 const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats);

			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) const;

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
