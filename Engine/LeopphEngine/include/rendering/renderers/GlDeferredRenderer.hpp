#pragma once

#include "GlRenderer.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/shaders/ShaderFamily.hpp"

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

			ShaderFamily m_GeometryShader{MakeShaderFamily("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GPass.vert", {}, "C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GPass.frag")};
			ShaderFamily m_LightShader{MakeShaderFamily("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Pos2DPassthrough.vert", {}, "C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/LightPass.frag")};
	};
}
