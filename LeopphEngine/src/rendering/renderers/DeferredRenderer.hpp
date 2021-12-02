#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../CubeShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../RenderTexture.hpp"
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
			void RenderGeometry(const Matrix4& camViewMat, const Matrix4& camProjMat);
			void RenderAmbientLight();
			void RenderDirectionalLights(const Matrix4& camViewMat, const Matrix4& camProjMat);
			void RenderSpotLights(const std::vector<const SpotLight*>& spotLights);
			void RenderPointLights(const std::vector<const PointLight*>& pointLights);
			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat);

			GeometryBuffer m_GBuffer;
			RenderTexture m_RenderTexture;

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
