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
		private:
			struct Gbuffer
			{
				GLuint framebuffer;
				GLuint colorAttachment;
			};


		public:
			auto Render() -> void override;

		private:
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

			Gbuffer m_Gbuffer;

			std::array<GLuint, 3> m_Ubos;

			ShaderFamily m_GeometryShader{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex},
					{shadersources::COMPOSITE_FRAG, ShaderType::Fragment}
				}
			};

			ShaderFamily m_LightShader{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex},
					{shadersources::COMPOSITE_FRAG, ShaderType::Fragment}
				}
			};
	};
}
