#include "RenderTexture.hpp"

#include "../windowing/WindowBase.hpp"

#include <glad/glad.h>

#include <array>


namespace leopph::internal
{
	RenderTexture::RenderTexture() :
		m_FramebufferName{},
		m_ColorTextureName{},
		m_DepthBufferName{},
		m_VertexArrayName{},
		m_VertexBufferName{},
		m_Resolution{Vector2{WindowBase::Get().Width(), WindowBase::Get().Height()} * WindowBase::Get().RenderMultiplier()}
	{
		glCreateFramebuffers(1, &m_FramebufferName);

		InitTextures(static_cast<unsigned>(m_Resolution[0]), static_cast<unsigned>(m_Resolution[1]));

		glCreateBuffers(1, &m_VertexBufferName);
		glNamedBufferStorage(m_VertexBufferName, QUAD_VERTICES.size() * sizeof(decltype(QUAD_VERTICES)::value_type), QUAD_VERTICES.data(), 0);

		glCreateVertexArrays(1, &m_VertexArrayName);
		glVertexArrayVertexBuffer(m_VertexArrayName, 0, m_VertexBufferName, 0, 5 * sizeof(decltype(QUAD_VERTICES)::value_type));

		glEnableVertexArrayAttrib(m_VertexArrayName, 0);
		glEnableVertexArrayAttrib(m_VertexArrayName, 1);

		glVertexArrayAttribFormat(m_VertexArrayName, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArrayName, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(decltype(QUAD_VERTICES)::value_type));

		glVertexArrayAttribBinding(m_VertexArrayName, 0, 0);
		glVertexArrayAttribBinding(m_VertexArrayName, 1, 0);
	}

	RenderTexture::~RenderTexture()
	{
		DeinitTextures();
		glDeleteFramebuffers(1, &m_FramebufferName);
		glDeleteVertexArrays(1, &m_VertexArrayName);
		glDeleteBuffers(1, &m_VertexBufferName);
	}

	auto RenderTexture::DrawToTexture() const -> void
	{
		BindAsRenderTarget();
		glBindVertexArray(m_VertexArrayName);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		UnbindAsRenderTarget();
	}

	auto RenderTexture::DrawToWindow() const -> void
	{
		glBlitNamedFramebuffer(m_FramebufferName, 0, 0, 0, static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]), 0, 0, static_cast<GLsizei>(WindowBase::Get().Width()), static_cast<GLsizei>(WindowBase::Get().Height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	auto RenderTexture::BindAsRenderTarget() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferName);
		glViewport(0, 0, static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]));
	}

	auto RenderTexture::UnbindAsRenderTarget() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(WindowBase::Get().Width()), static_cast<GLsizei>(WindowBase::Get().Height()));
	}

	auto RenderTexture::FramebufferName() const -> unsigned
	{
		return m_FramebufferName;
	}

	auto RenderTexture::Clear() const -> void
	{
		constexpr std::array clearColor{0.f, 0.f, 0.f, 1.f};
		glClearNamedFramebufferfv(m_FramebufferName, GL_COLOR, 0, clearColor.data());
		constexpr std::array clearDepth{1.f};
		glClearNamedFramebufferfv(m_FramebufferName, GL_DEPTH, 0, clearDepth.data());
	}

	auto RenderTexture::InitTextures(const unsigned width, const unsigned height) -> void
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorTextureName);
		glTextureStorage2D(m_ColorTextureName, 1, GL_RGB8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		glTextureParameteri(m_ColorTextureName, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ColorTextureName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glCreateRenderbuffers(1, &m_DepthBufferName);
		glNamedRenderbufferStorage(m_DepthBufferName, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

		glNamedFramebufferTexture(m_FramebufferName, GL_COLOR_ATTACHMENT0, m_ColorTextureName, 0);
		glNamedFramebufferRenderbuffer(m_FramebufferName, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferName);

		glNamedFramebufferDrawBuffer(m_FramebufferName, GL_COLOR_ATTACHMENT0);
	}

	auto RenderTexture::DeinitTextures() const -> void
	{
		glDeleteTextures(1, &m_ColorTextureName);
		glDeleteRenderbuffers(1, &m_DepthBufferName);
	}

	auto RenderTexture::OnEventReceived(EventParamType event) -> void
	{
		m_Resolution = event.NewResolution * event.NewResolutionMultiplier;
		DeinitTextures();
		InitTextures(static_cast<unsigned>(m_Resolution[0]), static_cast<unsigned>(m_Resolution[1]));
	}
}
