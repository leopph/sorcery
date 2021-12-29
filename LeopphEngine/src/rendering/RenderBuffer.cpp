#include "RenderBuffer.hpp"

#include "../windowing/WindowBase.hpp"


namespace leopph::internal
{
	RenderBuffer::RenderBuffer() :
		m_Framebuffer{},
		m_ColorBuffer{},
		m_StencilBuffer{},
		m_VertexArray{},
		m_VertexBuffer{},
		m_Resolution{Vector2{WindowBase::Get().Width(), WindowBase::Get().Height()} * WindowBase::Get().RenderMultiplier()}
	{
		glCreateFramebuffers(1, &m_Framebuffer);

		InitBuffers(static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]));

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


	RenderBuffer::~RenderBuffer()
	{
		DeinitBuffers();
		glDeleteFramebuffers(1, &m_Framebuffer);
		glDeleteVertexArrays(1, &m_VertexArray);
		glDeleteBuffers(1, &m_VertexBuffer);
	}


	auto RenderBuffer::DrawQuad() const -> void
	{
		BindAsRenderTarget();
		glBindVertexArray(m_VertexArray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		UnbindAsRenderTarget();
	}


	auto RenderBuffer::CopyColorToDefaultBuffer() const -> void
	{
		glBlitNamedFramebuffer(m_Framebuffer, 0, 0, 0, static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]), 0, 0, static_cast<GLsizei>(WindowBase::Get().Width()), static_cast<GLsizei>(WindowBase::Get().Height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}


	auto RenderBuffer::BindAsRenderTarget() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]));
	}


	auto RenderBuffer::UnbindAsRenderTarget() -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(WindowBase::Get().Width()), static_cast<GLsizei>(WindowBase::Get().Height()));
	}


	auto RenderBuffer::FramebufferName() const -> unsigned
	{
		return m_Framebuffer;
	}


	auto RenderBuffer::Clear() const -> void
	{
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, WindowBase::Get().ClearColor().Data().data());
		glClearNamedFramebufferiv(m_Framebuffer, GL_STENCIL, 0, &CLEAR_STENCIL);
	}


	auto RenderBuffer::InitBuffers(const GLsizei width, const GLsizei height) -> void
	{
		glCreateRenderbuffers(1, &m_ColorBuffer);
		glNamedRenderbufferStorage(m_ColorBuffer, GL_RGB8, width, height);

		glCreateRenderbuffers(1, &m_StencilBuffer);
		glNamedRenderbufferStorage(m_StencilBuffer, GL_STENCIL_INDEX8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBuffer);
		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBuffer);

		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);
	}


	auto RenderBuffer::DeinitBuffers() const -> void
	{
		glDeleteRenderbuffers(1, &m_ColorBuffer);
		glDeleteRenderbuffers(1, &m_StencilBuffer);
	}


	auto RenderBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Resolution = event.NewResolution * event.NewResolutionMultiplier;
		DeinitBuffers();
		InitBuffers(static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]));
	}
}
