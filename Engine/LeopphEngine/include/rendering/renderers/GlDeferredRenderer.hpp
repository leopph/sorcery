#pragma once

#include "DirLight.hpp"
#include "GlRenderer.hpp"
#include "Matrix.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/gl/GlGeometryBuffer.hpp"
#include "rendering/shaders/ShaderFamily.hpp"

#include <array>
#include <vector>


namespace leopph::internal
{
	class GlDeferredRenderer final : public GlRenderer
	{
		public:
			auto Render() -> void override;

		private:
			// Transparent forward pass.
			auto RenderTransparent(Matrix4 const& camViewMat,
			                       Matrix4 const& camProjMat,
			                       std::vector<RenderNode> const& renderNodes,
			                       DirectionalLight const* dirLight,
			                       std::vector<SpotLight const*> const& spotLights,
			                       std::vector<PointLight const*> const& pointLights) -> void;

			auto CreateGbuffer(GLsizei renderWidth, GLsizei renderHeight) -> void;
			auto DeleteGbuffer() const -> void;

			auto CreateUbos() -> void;
			auto DeleteUbos() const -> void;

			auto OnRenderResChange(Extent2D renderRes) -> void override;

		public:
			/* ############
			 * RULE OF FIVE
			 * ############ */

			GlDeferredRenderer();
			~GlDeferredRenderer() override;


		private:
			/* ############
			 * DATA MEMBERS
			 * ############ */

			GLuint m_GbufferFramebuffer;
			std::vector<GLuint> m_GbufferColorAttachments;

			std::array<GLuint, 3> m_Ubos;

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
