#pragma once

#include "GlRenderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../CubeShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../ScreenQuad.hpp"
#include "../ScreenRenderBuffer.hpp"
#include "../SpotShadowMap.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderFamily.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <memory>
#include <vector>


namespace leopph::internal
{
	class GlDeferredRenderer final : public GlRenderer
	{
		public:
			GlDeferredRenderer();

			auto Render() -> void override;

		private:
			// Fill the GeometryBuffer with geometry data.
			auto RenderGeometry(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderableData> const& renderables) -> void;

			// Draw all lights in the ScreenRenderBuffer.
			auto RenderLights(Matrix4 const& camViewMat,
			                  Matrix4 const& camProjMat,
			                  std::span<RenderableData const> renderables,
			                  std::span<SpotLight const*> spotLights,
			                  std::span<PointLight const*> pointLights) -> void;

			// Draw skybox in the empty parts of the ScreenRenderBuffer.
			auto RenderSkybox(Matrix4 const& camViewMat, Matrix4 const& camProjMat) -> void;

			// Draws into the dirlight shadow map, binds it to light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderDirShadowMap(DirectionalLight const* dirLight,
			                                      Matrix4 const& camViewInvMat,
			                                      Matrix4 const& camProjMat,
			                                      std::span<RenderableData const> renderables,
			                                      ShaderProgram& lightShader,
			                                      ShaderProgram& shadowShader,
			                                      GLuint nextTexUnit) const -> GLuint;

			// Draws into the first N shadow maps, binds them to the light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderSpotShadowMaps(std::span<SpotLight const* const> spotLights,
			                                        std::span<RenderableData const> renderables,
			                                        ShaderProgram& lightShader,
			                                        ShaderProgram& shadowShader,
			                                        std::size_t numShadows,
			                                        GLuint nextTexUnit) -> GLuint;

			// Draws into the first N shadow maps, binds them to the light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderPointShadowMaps(std::span<PointLight const* const> pointLights,
			                                         std::span<RenderableData const> renderables,
			                                         ShaderProgram& lightShader,
			                                         ShaderProgram& shadowShader,
			                                         std::size_t numShadows,
			                                         GLuint nextTexUnit) -> GLuint;

			GeometryBuffer m_GBuffer;

			ScreenRenderBuffer m_ScreenRenderBuffer;
			ScreenQuad m_ScreenQuad;

			ShaderFamily m_ShadowShader;
			ShaderFamily m_CubeShadowShader;

			ShaderFamily m_GeometryShader;
			ShaderFamily m_LightShader;
			ShaderFamily m_SkyboxShader;

			CascadedShadowMap m_DirShadowMap;
			std::vector<std::unique_ptr<SpotShadowMap>> m_SpotShadowMaps;
			std::vector<std::unique_ptr<CubeShadowMap>> m_PointShadowMaps;

			static constexpr int STENCIL_REF{0};
			static constexpr int STENCIL_AND_MASK{1};
	};
}
