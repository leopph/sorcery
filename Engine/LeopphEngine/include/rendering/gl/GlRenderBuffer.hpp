#pragma once

#include "EventReceiver.hpp"
#include "GlCore.hpp"
#include "GlFramebufferObject.hpp"
#include "GlRenderbufferObject.hpp"
#include "WindowEvent.hpp"


namespace leopph::internal
{
	// A set of color, depth, and stencil buffers the size of the window scaled by the render multiplier.
	class GlRenderBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			GlRenderBuffer();

			GlRenderBuffer(GlRenderBuffer const& other) = delete;
			auto operator=(GlRenderBuffer const& other) -> GlRenderBuffer& = delete;

			GlRenderBuffer(GlRenderBuffer&& other) = delete;
			auto operator=(GlRenderBuffer&& other) -> GlRenderBuffer& = delete;

			~GlRenderBuffer() noexcept override = default;

			// Clears the color, depth, and stencil buffers.
			auto Clear() const -> void;

			// Binds the buffers for writing.
			auto BindForWriting() const -> void;

			[[nodiscard]]
			auto Framebuffer() const noexcept -> GlFramebufferObject const&;

			[[nodiscard]]
			auto ColorBuffer() const noexcept -> GlRenderbufferObject const&;

			[[nodiscard]]
			auto DepthStencilBuffer() const noexcept -> GlRenderbufferObject const&;

			[[nodiscard]]
			auto Width() const noexcept -> int;

			[[nodiscard]]
			auto Height() const noexcept -> int;

		private:
			// Initializes the buffers to the current resolution.
			auto InitBuffers() noexcept -> void;

			// Updates the resolution and reconfigures buffers.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebufferObject m_Framebuffer;
			GlRenderbufferObject m_ColorBuffer;
			GlRenderbufferObject m_DepthStencilBuffer;
			int m_Width;
			int m_Height;

			static constexpr GLint CLEAR_STENCIL{1};
			static constexpr GLfloat CLEAR_DEPTH{1};
	};
}
