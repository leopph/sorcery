#pragma once

#include "GlRenderer.hpp"
#include "rendering/ShaderFamily.hpp"
#include "rendering/gl/GlCore.hpp"

#include <array>


namespace leopph::internal
{
	class GlDeferredRenderer final : public GlRenderer
	{
		struct Gbuffer
		{
			GLuint framebuffer;
			GLuint colorAttachment;
		};


		public:
			void Render() override;

		private:
			void CreateGbuffer(GLsizei renderWidth, GLsizei renderHeight);
			void DeleteGbuffer() const;

			void CreateUbos();
			void DeleteUbos() const;

			void OnRenderResChange(Extent2D renderRes) override;

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

			ShaderFamily m_GeometryShader{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GPass.vert", {}, "C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GPass.frag")};
			ShaderFamily m_LightShader{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Pos2DPassthrough.vert", {}, "C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/LightPass.frag")};
	};
}
