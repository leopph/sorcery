#include "ShadowMap.hpp"
#include <glad/glad.h>

namespace leopph::impl
{
	ShadowMap::ShadowMap(const Vector2& resolution) :
		id{ m_FrameBufferHandle }, m_FrameBufferHandle {}, m_DepthMapHandle{}
	{
		glCreateFramebuffers(1, &m_FrameBufferHandle);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthMapHandle);

		glTextureStorage2D(m_DepthMapHandle, 1, GL_DEPTH_COMPONENT, static_cast<GLsizei>(resolution[0]), static_cast<GLsizei>(resolution[1]));
		glTextureParameteri(m_DepthMapHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_DepthMapHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_DepthMapHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_DepthMapHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glNamedFramebufferTexture(m_FrameBufferHandle, GL_DEPTH_ATTACHMENT, m_DepthMapHandle, 0);
		glNamedFramebufferDrawBuffer(m_FrameBufferHandle, GL_NONE);
		glNamedFramebufferReadBuffer(m_FrameBufferHandle, GL_NONE);
	}

	ShadowMap::~ShadowMap()
	{
		glDeleteTextures(1, &m_DepthMapHandle);
		glDeleteFramebuffers(1, &m_FrameBufferHandle);
	}

	ShadowMap::ShadowMap(ShadowMap&& other) noexcept :
		id{ m_FrameBufferHandle }, m_FrameBufferHandle{ other.m_FrameBufferHandle }, m_DepthMapHandle{ other.m_DepthMapHandle }
	{
		other.m_FrameBufferHandle = 0;
		other.m_DepthMapHandle = 0;
	}

	ShadowMap& ShadowMap::operator=(ShadowMap&& other) noexcept
	{
		m_FrameBufferHandle = other.m_FrameBufferHandle;
		m_DepthMapHandle = other.m_DepthMapHandle;

		other.m_FrameBufferHandle = 0;
		other.m_DepthMapHandle = 0;

		return *this;
	}

	void ShadowMap::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferHandle);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void ShadowMap::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}
