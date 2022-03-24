#pragma once

#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "opengl/GlFramebuffer.hpp"
#include "opengl/GlRenderbuffer.hpp"
#include "opengl/OpenGl.hpp"


namespace leopph::internal
{
	// A set of color, depth, and stencil buffers the size of the window scaled by the render multiplier.
	class RenderBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			RenderBuffer();

			RenderBuffer(RenderBuffer const& other) = delete;
			auto operator=(RenderBuffer const& other) -> RenderBuffer& = delete;

			RenderBuffer(RenderBuffer&& other) = delete;
			auto operator=(RenderBuffer&& other) -> RenderBuffer& = delete;

			~RenderBuffer() noexcept override = default;

			// Clears the color, depth, and stencil buffers.
			auto Clear() const -> void;

			// Binds the buffers for writing.
			auto BindForWriting() const -> void;

			[[nodiscard]]
			auto Framebuffer() const noexcept -> GlFramebuffer const&;

			[[nodiscard]]
			auto ColorBuffer() const noexcept -> GlRenderbuffer const&;

			[[nodiscard]]
			auto DepthStencilBuffer() const noexcept -> GlRenderbuffer const&;

			[[nodiscard]]
			auto Width() const noexcept -> int;

			[[nodiscard]]
			auto Height() const noexcept -> int;

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
