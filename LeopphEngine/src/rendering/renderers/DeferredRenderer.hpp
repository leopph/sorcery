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

#include <vector>


namespace leopph::internal
{
	class DeferredRenderer final : public Renderer
	{
		public:
			DeferredRenderer();

			auto Render() -> void override;

		private:
			auto RenderGeometry(const Matrix4& camViewMat, const Matrix4& camProjMat, const std::vector<RenderableData>& renderables) -> void;
			auto RenderAmbientLight() -> void;
			auto RenderDirectionalLights(const Matrix4& camViewMat, const Matrix4& camProjMat, const std::vector<RenderableData>& renderables) -> void;
			auto RenderSpotLights(const std::vector<const SpotLight*>& spotLights, const std::vector<RenderableData>& renderables) -> void;
			auto RenderPointLights(const std::vector<const PointLight*>& pointLights, const std::vector<RenderableData>& renderables) -> void;
			auto RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) -> void;

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

			static constexpr int STENCIL_REF{0};
	};
}
