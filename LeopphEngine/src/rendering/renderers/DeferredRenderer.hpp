#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../ScreenTexture.hpp"
#include "../SpotLightShadowMap.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderProgram.hpp"

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

			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat);

			GeometryBuffer m_GBuffer;
			CascadedShadowMap m_DirShadowMap;
			ScreenTexture m_ScreenTexture;

			ShaderProgram m_TextureShader;
			ShaderProgram m_ShadowShader;
			ShaderProgram m_GPassObjectShader;
			ShaderProgram m_AmbientShader;
			ShaderProgram m_ShadowedDirLightShader;
			ShaderProgram m_UnshadowedDirLightShader;
			ShaderProgram m_SpotLightShader;
			ShaderProgram m_SkyboxShader;

			SpotLightShadowMap m_SpotShadowMap;
	};
}
