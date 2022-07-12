#pragma once

#include "DirLight.hpp"
#include "GlRenderer.hpp"
#include "Matrix.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/gl/GlGeometryBuffer.hpp"
#include "rendering/shaders/ShaderFamily.hpp"

#include <cstddef>
#include <span>
#include <vector>


namespace leopph::internal
{
	class GlDeferredRenderer final : public GlRenderer
	{
		public:
			GlDeferredRenderer();

			auto Render() -> void override;

		private:
			// Fill the GlGeometryBuffer with geometry data.
			auto GeometryPass(std::vector<RenderNode> const& renderNodes, Matrix4 const& viewProjMat, GLsizei renderWidth, GLsizei renderHeight) -> void;

			// Draw all lights in the GlRenderBuffer.
			auto RenderLights(Matrix4 const& camViewMat,
			                  Matrix4 const& camProjMat,
			                  std::span<RenderNode const> renderNodes,
			                  std::span<SpotLight const*> spotLights,
			                  std::span<PointLight const*> pointLights) -> void;

			// Draw skybox in the empty parts of the GlRenderBuffer.
			auto RenderSkybox(Matrix4 const& camViewMat, Matrix4 const& camProjMat) -> void;

			// Draws into the dirlight shadow map, binds it to light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderDirShadowMap(DirectionalLight const* dirLight,
			                                      Matrix4 const& camViewInvMat,
			                                      Matrix4 const& camProjMat,
			                                      std::span<RenderNode const> renderNodes,
			                                      ShaderProgram& lightShader,
			                                      ShaderProgram& shadowShader,
			                                      GLuint nextTexUnit) const -> GLuint;

			// Draws into the first N shadow maps, binds them to the light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderSpotShadowMaps(std::span<SpotLight const* const> spotLights,
			                                        std::span<RenderNode const> renderNodes,
			                                        ShaderProgram& lightShader,
			                                        ShaderProgram& shadowShader,
			                                        std::size_t numShadows,
			                                        GLuint nextTexUnit) -> GLuint;

			// Draws into the first N shadow maps, binds them to the light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderPointShadowMaps(std::span<PointLight const* const> pointLights,
			                                         std::span<RenderNode const> renderNodes,
			                                         ShaderProgram& lightShader,
			                                         ShaderProgram& shadowShader,
			                                         std::size_t numShadows,
			                                         GLuint nextTexUnit) -> GLuint;

			// Transparent forward pass.
			auto RenderTransparent(Matrix4 const& camViewMat,
			                       Matrix4 const& camProjMat,
			                       std::vector<RenderNode> const& renderNodes,
			                       DirectionalLight const* dirLight,
			                       std::vector<SpotLight const*> const& spotLights,
			                       std::vector<PointLight const*> const& pointLights) -> void;

			auto CreateGbuffer(GLsizei renderWidth, GLsizei renderHeight) -> void;
			auto DeleteGbuffer() const -> void;

			auto OnRenderResChange(Extent2D renderRes) -> void override;

			GLuint m_GbufferFramebuffer;
			std::vector<GLuint> m_GbufferColorAttachments;

			GlGeometryBuffer m_GBuffer;

			ShaderFamily m_GeometryShader{
				{
					{ShaderFamily::GeometryPassVertSrc, ShaderType::Vertex},
					{ShaderFamily::GeometryPassFragSrc, ShaderType::Fragment}
				}
			};

			ShaderFamily m_LightShader{
				{
					{ShaderFamily::Pos2DPassthroughVertSrc, ShaderType::Vertex},
					{ShaderFamily::LightPassFragSrc, ShaderType::Fragment}
				}
			};
	};
}
