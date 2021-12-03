#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../shaders/ShaderFamily.hpp"
#include "../../math/Matrix.hpp"



namespace leopph::impl
{
	class ForwardRenderer final : public Renderer
	{
		public:
			ForwardRenderer();
			void Render() override;


		private:
			void RenderShadedObjects(const Matrix4& camViewMat, const Matrix4& camProjMat,
									 const std::vector<const PointLight*>& pointLights, const std::vector<const SpotLight*>& spotLights);
			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat);

			ShaderFamily m_ObjectShader;
			ShaderFamily m_ObjectShaderInstanced;

			ShaderFamily m_ShadowShader;
			ShaderFamily m_ShadowShaderInstanced;

			ShaderFamily m_SkyboxShader;

			CascadedShadowMap m_DirLightShadowMap;
	};
}
