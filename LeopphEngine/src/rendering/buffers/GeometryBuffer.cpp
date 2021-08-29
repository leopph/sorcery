#include "GeometryBuffer.hpp"

#include "../../math/Vector.hpp"
#include "../../windowing/window.h"

#include <glad/glad.h>

#include <cstddef>


namespace leopph::impl
{
	GeometryBuffer::GeometryBuffer() :
		positionTextureName{ m_Textures[Position] },
		normalTextureName{ m_Textures[Normal] },
		ambientTextureName{ m_Textures[Ambient] },
		diffuseTextureName{ m_Textures[Diffuse] },
		specularTextureName{ m_Textures[Specular] },
		shineTextureName{ m_Textures[Shine] },
		m_Textures{}, m_DepthBuffer{}
	{
		SetUpBuffers(Vector2{ Window::Get().Width(), Window::Get().Height() });
	}


	GeometryBuffer::~GeometryBuffer()
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
		glDeleteRenderbuffers(1, &m_DepthBuffer);
	}


	void GeometryBuffer::Clear() const
	{
		for (std::size_t i = 0; i < m_Textures.size(); i++)
		{
			glClearNamedFramebufferfv(name, GL_COLOR, static_cast<GLint>(i), Vector4{ 0, 0, 0, 1 }.Data());
		}

		glClearNamedFramebufferfv(name, GL_DEPTH, 0, std::array<GLfloat, 1>{ 1 }.data());
	}


	void GeometryBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, name);
	}


	void GeometryBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	void GeometryBuffer::SetUpBuffers(const Vector2& res)
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
		glDeleteRenderbuffers(1, &m_DepthBuffer);

		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_Textures.size()), m_Textures.data());

		glTextureStorage2D(m_Textures[Position], 1, GL_RGB32F, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[Position], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[Position], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[Normal], 1, GL_RGB32F, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[Normal], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[Normal], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[Ambient], 1, GL_RGB8, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[Ambient], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[Ambient], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[Diffuse], 1, GL_RGB8, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[Diffuse], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[Diffuse], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[Specular], 1, GL_RGB8, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[Specular], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[Specular], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[Shine], 1, GL_R32F, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[Shine], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[Shine], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glCreateRenderbuffers(1, &m_DepthBuffer);
		glNamedRenderbufferStorage(m_DepthBuffer, GL_DEPTH_COMPONENT, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));

		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT0, m_Textures[Position], 0);
		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT1, m_Textures[Normal], 0);
		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT2, m_Textures[Ambient], 0);
		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT3, m_Textures[Diffuse], 0);
		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT4, m_Textures[Specular], 0);
		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT5, m_Textures[Shine], 0);
		glNamedFramebufferRenderbuffer(name, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer);

		glNamedFramebufferDrawBuffers(name, static_cast<GLsizei>(m_Textures.size()), std::array<GLenum, 6>{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 }.data());
	}


	void GeometryBuffer::OnEventReceived(EventParamType event)
	{
		SetUpBuffers(event.newResolution);
	}
}
