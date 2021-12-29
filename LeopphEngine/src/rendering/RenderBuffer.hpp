#pragma once

#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <array>


namespace leopph::internal
{
	class RenderBuffer final : public EventReceiver<ScreenResolutionEvent>
	{
		public:
			RenderBuffer();

			RenderBuffer(const RenderBuffer& other) = delete;
			auto operator=(const RenderBuffer& other) -> RenderBuffer& = delete;

			RenderBuffer(RenderBuffer&& other) = delete;
			auto operator=(RenderBuffer&& other) -> RenderBuffer& = delete;

			~RenderBuffer() override;

			// Draws a full screen quad on its framebuffer.
			auto DrawQuad() const -> void;
			auto CopyColorToDefaultBuffer() const -> void;

			auto BindAsRenderTarget() const -> void;
			static auto UnbindAsRenderTarget() -> void;

			auto Clear() const -> void;

			[[nodiscard]] constexpr auto FramebufferName() const noexcept;

		private:
			auto OnEventReceived(EventParamType event) -> void override;
			auto InitBuffers(GLsizei width, GLsizei height) -> void;
			auto DeinitBuffers() const -> void;

			GLuint m_Framebuffer;
			GLuint m_ColorBuffer;
			GLuint m_StencilBuffer;
			GLuint m_VertexArray;
			GLuint m_VertexBuffer;
			Vector2 m_Resolution;
			
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
