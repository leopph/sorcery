#pragma once

#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "opengl/GlFramebuffer.hpp"
#include "opengl/GlTexture.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/gl.h>


namespace leopph::internal
{
	class GeometryBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			GeometryBuffer();

			GeometryBuffer(GeometryBuffer const&) = delete;
			GeometryBuffer(GeometryBuffer&&) = delete;

			auto operator=(GeometryBuffer const&) -> GeometryBuffer& = delete;
			auto operator=(GeometryBuffer&&) -> GeometryBuffer& = delete;

			~GeometryBuffer() noexcept override = default;

			[[nodiscard]]
			auto Framebuffer() const noexcept -> GlFramebuffer const&;

			[[nodiscard]]
			auto NormalColorGlossTexture() const noexcept -> GlTexture<GlTextureType::T2D> const&;

			[[nodiscard]]
			auto DepthStencilTexture() const noexcept -> GlTexture<GlTextureType::T2D> const&;

			// Clears all buffers.
			auto Clear() const -> void;

			// Binds the buffers for writing.
			auto BindForWriting() const -> void;

			// Bind all textures and returns the next available texture unit after binding.
			[[nodiscard]]
			auto BindForReading(ShaderProgram& shader, GLuint texUnit) const -> GLuint;

			auto CopyStencilData(GLuint bufferName) const -> void;

		private:
			// Configures the buffers for the current resolution.
			auto ConfigBuffers() noexcept -> void;

			// Reconfigures the buffers to the new resolution.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebuffer m_Framebuffer;
			GlTexture<GlTextureType::T2D> m_NormClrGlossTexture;
			GlTexture<GlTextureType::T2D> m_DepthStencilTexture;
			int m_Width;
			int m_Height;
	};
}
