#include "ShadowMap.hpp"

#include "../windowing/window.h"

#include <glad/glad.h>



namespace leopph::impl
{
	ShadowMap::ShadowMap(const std::size_t resolution) :
		Id{m_Fbo},
		m_Resolution{resolution},
		m_Fbo{},
		m_DepthTex{},
		m_CurrentBindIndex{}
	{
		glCreateFramebuffers(1, &m_Fbo);
		glNamedFramebufferDrawBuffer(m_Fbo, GL_NONE);
		glNamedFramebufferReadBuffer(m_Fbo, GL_NONE);

		Init();
	}


	ShadowMap::~ShadowMap()
	{
		Deinit();
		glDeleteFramebuffers(1, &m_Fbo);
	}


	ShadowMap::ShadowMap(ShadowMap&& other) noexcept :
		Id{m_Fbo},
		m_Fbo{other.m_Fbo},
		m_DepthTex{other.m_DepthTex},
		m_CurrentBindIndex{other.m_CurrentBindIndex}
	{
		other.m_Fbo = 0;
		other.m_DepthTex = 0;
	}


	ShadowMap& ShadowMap::operator=(ShadowMap&& other) noexcept
	{
		m_Fbo = other.m_Fbo;
		m_DepthTex = other.m_DepthTex;
		m_CurrentBindIndex = other.m_CurrentBindIndex;

		other.m_Fbo = 0;
		other.m_DepthTex = 0;
		other.m_CurrentBindIndex = 0;

		return *this;
	}


	void ShadowMap::BindForWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
		glViewport(0, 0, static_cast<GLsizei>(m_Resolution), static_cast<GLsizei>(m_Resolution));
		Clear();
	}


	void ShadowMap::UnbindFromWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const auto& window{Window::Get()};
		glViewport(0, 0, static_cast<GLsizei>(window.Width()), static_cast<GLsizei>(window.Height()));
	}


	int ShadowMap::BindForReading(const int textureUnit) const
	{
		m_CurrentBindIndex = textureUnit;
		glBindTextureUnit(static_cast<GLuint>(m_CurrentBindIndex), m_DepthTex);
		return m_CurrentBindIndex + 1;
	}


	void ShadowMap::UnbindFromReading() const
	{
		glBindTextureUnit(m_CurrentBindIndex, 0);
	}


	void ShadowMap::Clear() const
	{
		constexpr float clearValue{1};
		glClearNamedFramebufferfv(m_Fbo, GL_DEPTH, 0, &clearValue);
	}


	void ShadowMap::Init()
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthTex);

		glTextureStorage2D(m_DepthTex, 1, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(m_Resolution), static_cast<GLsizei>(m_Resolution));
		glTextureParameteri(m_DepthTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glNamedFramebufferTexture(m_Fbo, GL_DEPTH_ATTACHMENT, m_DepthTex, 0);
	}


	void ShadowMap::Deinit() const
	{
		glDeleteTextures(1, &m_DepthTex);
	}

}
