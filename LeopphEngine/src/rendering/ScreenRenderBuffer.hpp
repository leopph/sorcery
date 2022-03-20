#pragma once

#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "opengl/GlFramebuffer.hpp"
#include "opengl/GlRenderbuffer.hpp"

#include <glad/gl.h>


namespace leopph::internal
{
	// A set of color, depth, and stencil buffers the size of the window scaled by the render multiplier.
	class ScreenRenderBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			ScreenRenderBuffer();

			ScreenRenderBuffer(ScreenRenderBuffer const& other) = delete;
			auto operator=(ScreenRenderBuffer const& other) -> ScreenRenderBuffer& = delete;

			ScreenRenderBuffer(ScreenRenderBuffer&& other) = delete;
			auto operator=(ScreenRenderBuffer&& other) -> ScreenRenderBuffer& = delete;

			~ScreenRenderBuffer() noexcept override = default;

			// Clears the color, depth, and stencil buffers.
			auto Clear() const -> void;

			// Binds the buffers for writing.
			auto BindForWriting() const -> void;

			// Copies the contents of the color buffer to the default framebuffer.
			auto CopyColorToDefaultFramebuffer() const -> void;

			[[nodiscard]]
			auto Framebuffer() const noexcept -> GlFramebuffer const&;

			[[nodiscard]]
			auto ColorBuffer() const noexcept -> GlRenderbuffer const&;

			[[nodiscard]]
			auto DepthStencilBuffer() const noexcept -> GlRenderbuffer const&;

		private:
			// Initializes the buffers to the current resolution.
			auto InitBuffers() noexcept -> void;

			// Updates the resolution and reconfigures buffers.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebuffer m_Framebuffer;
			GlRenderbuffer m_ColorBuffer;
			GlRenderbuffer m_DepthStencilBuffer;
			int m_Width;
			int m_Height;

			static constexpr GLint CLEAR_STENCIL{1};
			static constexpr GLfloat CLEAR_DEPTH{1};
	};
}
