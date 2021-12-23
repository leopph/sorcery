#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderFamily.hpp"


namespace leopph::internal
{
	class ForwardRenderer final : public Renderer
	{
		public:
			ForwardRenderer();
			void Render() override;

		private:
			void RenderShadedObjects(const Matrix4& camViewMat, const Matrix4& camProjMat, const std::vector<RenderableData>& renderables, const DirectionalLight* dirLight, const std::vector<const SpotLight*>& spotLights, const std::vector<const PointLight*>& pointLights);
			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat);

			ShaderFamily m_ObjectShader;
			ShaderFamily m_ShadowShader;
			ShaderFamily m_SkyboxShader;

			CascadedShadowMap m_DirLightShadowMap;
	};
}
