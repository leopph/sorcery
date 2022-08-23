#pragma once

#include "Extent.hpp"
#include "GlCore.hpp"
#include "GlRenderer.hpp"
#include "ShaderFamily.hpp"

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

			ShaderFamily mGbufferShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Gbuffer.shader")};
			ShaderFamily mLightingShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/DeferredLighting.shader")};
	};
}
