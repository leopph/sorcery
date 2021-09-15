#pragma once

#include "Renderer.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/DirShadowMapShader.hpp"
#include "../shaders/ForwardObjectShader.hpp"
#include "../shaders/SkyboxShader.hpp"

#include <optional>



namespace leopph::impl
{
	class ForwardRenderer final : public Renderer
	{
		public:
			ForwardRenderer();
			void Render() override;

		private:
			[[nodiscard]] std::optional<Matrix4> RenderDirectionalShadowMap(const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats) const;

			static void RenderPointShadowMaps(const std::vector<const PointLight*>& pointLights,
									   const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats);

			void RenderShadedObjects(const Matrix4& camViewMat,
									 const Matrix4& camProjMat,
									 const std::optional<Matrix4>& lightTransformMat,
									 const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats,
									 const std::vector<const PointLight*>& pointLights,
									 const std::vector<const SpotLight*>& spotLights) const;

			void RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) const;

			ForwardObjectShader m_ObjectShader;
			SkyboxShader m_SkyboxShader;
			DirShadowMapShader m_DirectionalShadowMapShader;
	};
}
