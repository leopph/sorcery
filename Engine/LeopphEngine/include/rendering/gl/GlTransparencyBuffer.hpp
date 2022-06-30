#pragma once

#include "EventReceiver.hpp"
#include "GlFramebufferObject.hpp"
#include "GlRenderbufferObject.hpp"
#include "GlTextureObject.hpp"
#include "WindowEvent.hpp"
#include "rendering/shaders/ShaderProgram.hpp"


namespace leopph::internal
{
	// A window res * render multiplier sized render target containing an
	// accumulation and a revealage  buffer for transparent rendering.
	// It also takes an external readonly depth buffer.
	class GlTransparencyBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			// odepthBuffer shall not be nullptr.
			explicit GlTransparencyBuffer(GlRenderbufferObject const* depthBuffer);

			GlTransparencyBuffer(GlTransparencyBuffer const& other) = delete;
			auto operator=(GlTransparencyBuffer const& other) -> GlTransparencyBuffer& = delete;

			GlTransparencyBuffer(GlTransparencyBuffer&& other) noexcept = delete;
			auto operator=(GlTransparencyBuffer&& other) noexcept -> GlTransparencyBuffer& = delete;

			~GlTransparencyBuffer() noexcept override = default;

			auto Clear() const -> void;

			auto BindForWriting() const -> void;

			auto BindForReading(ShaderProgram& shader, GLint nextFreeTexUnit) const -> GLint;

		private:
			// Configures the render targets to the current resolution.
			auto ConfigBuffers() -> void;

			// Updates the resolution and reconfigures the buffers.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebufferObject m_Framebuffer;
			GlTextureObject<GlTextureType::T2D> m_AccumBuffer;
			GlTextureObject<GlTextureType::T2D> m_RevealBuffer;
			GlRenderbufferObject const* m_DepthBuffer;
			int m_Width;
			int m_Height;
	};
}
