#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../ScreenTexture.hpp"
#include "../SpotLightShadowMap.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/DeferredAmbLightShader.hpp"
#include "../shaders/DeferredDirLightShader.hpp"
#include "../shaders/DeferredGeometryShader.hpp"
#include "../shaders/DeferredSpotLightShader.hpp"
#include "../shaders/ShadowMapShader.hpp"
#include "../shaders/SkyboxShader.hpp"
#include "../shaders/TextureShader.hpp"

#include <unordered_map>
#include <utility>
#include <vector>



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

			void RenderSpotLights(const std::vector<const SpotLight*>& spotLights,
			                      const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats) const;

			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) const;

			GeometryBuffer m_GBuffer;
			CascadedShadowMap m_DirShadowMap;
			ScreenTexture m_ScreenTexture;

			TextureShader m_TextureShader;
			ShadowMapShader m_ShadowShader;
			DeferredGeometryShader m_GPassObjectShader;
			DeferredAmbLightShader m_AmbientShader;
			DeferredDirLightShader m_DirLightShader;
			DeferredSpotLightShader m_SpotLightShader;
			SkyboxShader m_SkyboxShader;

			SpotLightShadowMap m_SpotShadowMap;
	};
}
