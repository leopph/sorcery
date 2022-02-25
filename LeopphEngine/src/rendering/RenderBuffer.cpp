#include "RenderBuffer.hpp"

#include "../windowing/WindowImpl.hpp"


namespace leopph::internal
{
	RenderBuffer::RenderBuffer() :
		m_Framebuffer{},
		m_ColorBuffer{},
		m_StencilBuffer{},
		m_VertexArray{},
		m_VertexBuffer{},
		m_Res{
			[]
			{
				const auto& window{Window::Instance()};
				const Vector2 displayRes{window->Width(), window->Height()};
				const auto renderRes{displayRes * window->RenderMultiplier()};
				return ResType{renderRes[0], renderRes[1]};
			}()
		}
	{
		glCreateFramebuffers(1, &m_Framebuffer);

		InitBuffers();

		glCreateBuffers(1, &m_VertexBuffer);
		glNamedBufferStorage(m_VertexBuffer, QUAD_VERTICES.size() * sizeof(decltype(QUAD_VERTICES)::value_type), QUAD_VERTICES.data(), 0);

		glCreateVertexArrays(1, &m_VertexArray);
		glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, 5 * sizeof(decltype(QUAD_VERTICES)::value_type));

		glEnableVertexArrayAttrib(m_VertexArray, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(decltype(QUAD_VERTICES)::value_type));

		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
	}


	RenderBuffer::~RenderBuffer() noexcept
	{
		DeinitBuffers();
		glDeleteFramebuffers(1, &m_Framebuffer);
		glDeleteVertexArrays(1, &m_VertexArray);
		glDeleteBuffers(1, &m_VertexBuffer);
	}


	auto RenderBuffer::BindForWritingAndClearColor() const -> void
	{
		BindForWriting();
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, static_cast<WindowImpl*>(Window::Instance())->ClearColor().Data().data());
	}


	auto RenderBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Res[0], m_Res[1]);
	}


	auto RenderBuffer::DrawScreenQuad() const -> void
	{
		BindForWritingAndClearColor();

		glBindVertexArray(m_VertexArray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}


	auto RenderBuffer::CopyColorToDefaultFramebuffer() const -> void
	{
		const auto& window{Window::Instance()};
		glBlitNamedFramebuffer(m_Framebuffer, 0, 0, 0, m_Res[0], m_Res[1], 0, 0, static_cast<GLsizei>(window->Width()), static_cast<GLsizei>(window->Height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}


	auto RenderBuffer::OnEventReceived(EventParamType event) -> void
	{
		const auto renderRes{event.Resolution * event.RenderMultiplier};
		m_Res = ResType{renderRes[0], renderRes[1]};
		DeinitBuffers();
		InitBuffers();
	}


	auto RenderBuffer::InitBuffers() noexcept -> void
	{
		glCreateRenderbuffers(1, &m_ColorBuffer);
		glNamedRenderbufferStorage(m_ColorBuffer, GL_RGB8, m_Res[0], m_Res[1]);

		glCreateRenderbuffers(1, &m_StencilBuffer);
		glNamedRenderbufferStorage(m_StencilBuffer, GL_DEPTH24_STENCIL8, m_Res[0], m_Res[1]); // This could use GL_STENCIL_INDEX# format but Nvidia decided to go against the standard and refuse to work with it so here we are wasting memory.

		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBuffer);
		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBuffer);

		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);
	}


	auto RenderBuffer::DeinitBuffers() const noexcept -> void
	{
		glDeleteRenderbuffers(1, &m_ColorBuffer);
		glDeleteRenderbuffers(1, &m_StencilBuffer);
	}
}
