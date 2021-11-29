#pragma once

#include "Renderer.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderFamily.hpp"



namespace leopph::impl
{
	class ForwardRenderer final : public Renderer
	{
		public:
			ForwardRenderer();
			void Render() override;

		private:
			void RenderShadedObjects(const Matrix4& camViewMat,
									 const Matrix4& camProjMat,
									 const std::unordered_map<ModelResource*, std::vector< std::pair<Matrix4, Matrix4>>>& modelsAndMats,
									 const std::vector<const PointLight*>& pointLights,
									 const std::vector<const SpotLight*>& spotLights);

			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat);

			ShaderFamily m_ObjectShader;
			ShaderFamily m_SkyboxShader;
			ShaderFamily m_ShadowShader;
	};
}
