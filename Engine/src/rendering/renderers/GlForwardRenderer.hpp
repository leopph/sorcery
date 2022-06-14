#pragma once

#include "GlRenderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../ScreenQuad.hpp"
#include "../RenderBuffer.hpp"
#include "../TransparencyBuffer.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderFamily.hpp"


namespace leopph::internal
{
	class GlForwardRenderer final : public GlRenderer
	{
		public:
			GlForwardRenderer();

			auto Render() -> void override;

		private:
			auto RenderOpaque(Matrix4 const& camViewMat,
			                         Matrix4 const& camProjMat,
			                         std::vector<RenderableData> const& renderables,
			                         DirectionalLight const* dirLight,
			                         std::vector<SpotLight const*> const& spotLights,
			                         std::vector<PointLight const*> const& pointLights) -> void;

			auto RenderTransparent(Matrix4 const& camViewMat,
			                         Matrix4 const& camProjMat,
			                         std::vector<RenderableData> const& renderables,
			                         DirectionalLight const* dirLight,
			                         std::vector<SpotLight const*> const& spotLights,
			                         std::vector<PointLight const*> const& pointLights) -> void;

			auto Compose() -> void;

			auto RenderSkybox(Matrix4 const& camViewMat,
			                  Matrix4 const& camProjMat) -> void;

			ShaderFamily m_ObjectShader;
			ShaderFamily m_ShadowShader;
			ShaderFamily m_SkyboxShader;
			ShaderFamily m_TranspCompositeShader;

			CascadedShadowMap m_DirLightShadowMap;

			RenderBuffer m_RenderBuffer;
			TransparencyBuffer m_TransparencyBuffer;
			ScreenQuad m_ScreenQuad;
	};
}
