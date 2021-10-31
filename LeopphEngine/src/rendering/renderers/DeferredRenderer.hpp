#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../CubeShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../ScreenTexture.hpp"
#include "../SpotLightShadowMap.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderFamily.hpp"

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
			                    const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats);

			void RenderAmbientLight();

			void RenderDirectionalLights(const Matrix4& camViewMat,
			                             const Matrix4& camProjMat,
			                             const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats);

			void RenderSpotLights(const std::vector<const SpotLight*>& spotLights,
			                      const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats);

			void RenderPointLights(const std::vector<const PointLight*>& pointLights,
			                       const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats);


			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat);

			GeometryBuffer m_GBuffer;
			ScreenTexture m_ScreenTexture;

			ShaderFamily m_TextureShader;

			ShaderFamily m_ShadowShader;
			ShaderFamily m_CubeShadowShader;

			ShaderFamily m_GeometryShader;

			ShaderFamily m_SkyboxShader;

			ShaderFamily m_AmbientShader;
			ShaderFamily m_DirLightShader;
			ShaderFamily m_SpotLightShader;
			ShaderFamily m_PointLightShader;

			CascadedShadowMap m_DirShadowMap;
			SpotLightShadowMap m_SpotShadowMap;
			CubeShadowMap m_PointShadowMap;
	};
}