#pragma once

#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "opengl/GlFramebuffer.hpp"
#include "opengl/GlRenderbuffer.hpp"
#include "opengl/GlTexture.hpp"
#include "shaders/ShaderProgram.hpp"


namespace leopph::internal
{
	// A window res * render multiplier sized render target containing an
	// accumulation and a revealage  buffer for transparent rendering.
	// It also takes an external readonly depth buffer.
	class TransparencyBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			// odepthBuffer shall not be nullptr.
			explicit TransparencyBuffer(GlRenderbuffer const* depthBuffer);

			TransparencyBuffer(TransparencyBuffer const& other) = delete;
			auto operator=(TransparencyBuffer const& other) -> TransparencyBuffer& = delete;

			TransparencyBuffer(TransparencyBuffer&& other) noexcept = delete;
			auto operator=(TransparencyBuffer&& other) noexcept -> TransparencyBuffer& = delete;

			~TransparencyBuffer() noexcept override = default;

			auto Clear() const -> void;

			auto BindForWriting() const -> void;

			auto BindForReading(ShaderProgram& shader, GLint nextFreeTexUnit) const -> GLint;

		private:
			// Configures the render targets to the current resolution.
			auto ConfigBuffers() -> void;

			// Updates the resolution and reconfigures the buffers.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebuffer m_Framebuffer;
			GlTexture<GlTextureType::T2D> m_AccumBuffer;
			GlTexture<GlTextureType::T2D> m_RevealBuffer;
			GlRenderbuffer const* m_DepthBuffer;
			int m_Width;
			int m_Height;
	};
}
