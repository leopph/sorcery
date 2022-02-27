#pragma once

#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/gl.h>

#include <array>


namespace leopph::internal
{
	class RenderBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			RenderBuffer();

			RenderBuffer(const RenderBuffer& other) = delete;
			auto operator=(const RenderBuffer& other) -> RenderBuffer& = delete;

			RenderBuffer(RenderBuffer&& other) = delete;
			auto operator=(RenderBuffer&& other) -> RenderBuffer& = delete;

			~RenderBuffer() noexcept override;

			// Binds the buffers for writing and clears the color values.
			auto BindForWritingAndClearColor() const -> void;

			// Binds the buffers for writing but does not clear any values.
			auto BindForWriting() const -> void;

			// Draws a full screen quad onto its framebuffer.
			auto DrawScreenQuad() const -> void;

			// Copies the contents of the color buffer to the default framebuffer.
			auto CopyColorToDefaultFramebuffer() const -> void;

			[[nodiscard]] constexpr auto FramebufferName() const noexcept;

		private:
			using ResType = Vector<GLsizei, 2>;

			// Initializes the buffers to the current resolution.
			auto InitBuffers() noexcept -> void;
			// Destroys the buffers.
			auto DeinitBuffers() const noexcept -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			GLuint m_Framebuffer;
			GLuint m_ColorBuffer;
			GLuint m_StencilBuffer;
			GLuint m_VertexArray;
			GLuint m_VertexBuffer;
			ResType m_Res;

			static constexpr GLint CLEAR_STENCIL{1};
			static constexpr std::array<float, 20> QUAD_VERTICES
			{
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
	};


	constexpr auto RenderBuffer::FramebufferName() const noexcept
	{
		return m_Framebuffer;
	}
}
