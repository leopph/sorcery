#include "GeometryBuffer.hpp"

#include "../windowing/Window.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <cstddef>



namespace leopph::impl
{
	constexpr int GeometryBuffer::s_BindFillValue{-1};


	GeometryBuffer::GeometryBuffer() :
		m_Textures{},
		m_BindIndices{},
		m_DepthBuffer{},
		m_FrameBuffer{},
		m_Resolution{Vector2{Window::Get().Width(), Window::Get().Height()}}
	{
		std::ranges::fill(m_BindIndices, s_BindFillValue);
		glCreateFramebuffers(1, &m_FrameBuffer);
		SetUpBuffers(m_Resolution);
	}


	GeometryBuffer::~GeometryBuffer()
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
		glDeleteRenderbuffers(1, &m_DepthBuffer);
		glDeleteFramebuffers(1, &m_FrameBuffer);
	}


	void GeometryBuffer::Clear() const
	{
		for (std::size_t i = 0; i < m_Textures.size(); i++)
		{
			glClearNamedFramebufferfv(m_FrameBuffer, GL_COLOR, static_cast<GLint>(i), Vector4{static_cast<Vector3>(Window::Get().Background())}.Data().data());
		}

		glClearNamedFramebufferfv(m_FrameBuffer, GL_DEPTH, 0, std::array<GLfloat, 1>{1}.data());
	}


	void GeometryBuffer::BindForWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
		glViewport(0, 0, static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]));
	}


	void GeometryBuffer::UnbindFromWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const auto& window{Window::Get()};
		glViewport(0, 0, static_cast<GLsizei>(window.Width()), static_cast<GLsizei>(window.Height()));
	}


	int GeometryBuffer::BindForReading(ShaderProgram& shader, const TextureType type, int texUnit) const
	{
		glBindTextureUnit(static_cast<unsigned>(texUnit), m_Textures[type]);

		auto uniformName{""};

		switch (type)
		{
			case Position:
				uniformName = "u_PositionTexture";
				break;

			case Normal:
				uniformName = "u_NormalTexture";
				break;

			case Ambient:
				uniformName = "u_AmbientTexture";
				break;

			case Diffuse:
				uniformName = "u_DiffuseTexture";
				break;

			case Specular:
				uniformName = "u_SpecularTexture";
				break;

			case Shine:
				uniformName = "u_ShineTexture";
				break;
		}

		shader.SetUniform(uniformName, texUnit);

		m_BindIndices[type] = texUnit;
		return ++texUnit;
	}


	int GeometryBuffer::BindForReading(ShaderProgram& shader, int texUnit) const
	{
		for (std::size_t i = 0; i < m_Textures.size(); ++i)
		{
			texUnit = BindForReading(shader, static_cast<TextureType>(i), texUnit);
		}

		return texUnit;
	}


	void GeometryBuffer::UnbindFromReading(const TextureType type) const
	{
		if (m_BindIndices[type] != s_BindFillValue)
		{
			glBindTextureUnit(m_BindIndices[type], 0);
			m_BindIndices[type] = s_BindFillValue;
		}
	}


	void GeometryBuffer::UnbindFromReading() const
	{
		for (std::size_t i = 0; i < m_Textures.size(); ++i)
		{
			UnbindFromReading(static_cast<TextureType>(i));
		}
	}


	void GeometryBuffer::CopyDepthData(const unsigned bufferName, const Vector2& resolution) const
	{
		glBlitNamedFramebuffer(m_FrameBuffer, bufferName, 0, 0, static_cast<GLint>(resolution[0]), static_cast<GLint>(resolution[1]), 0, 0, static_cast<GLint>(resolution[0]), static_cast<GLint>(resolution[1]), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}


	void GeometryBuffer::SetUpBuffers(const Vector2& res)
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
		glDeleteRenderbuffers(1, &m_DepthBuffer);

		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_Textures.size()), m_Textures.data());

		glTextureStorage2D(m_Textures[Position], 1, GL_RGBA32F, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
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

		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT0, m_Textures[Position], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT1, m_Textures[Normal], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT2, m_Textures[Ambient], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT3, m_Textures[Diffuse], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT4, m_Textures[Specular], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT5, m_Textures[Shine], 0);
		glNamedFramebufferRenderbuffer(m_FrameBuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer);

		glNamedFramebufferDrawBuffers(m_FrameBuffer, static_cast<GLsizei>(m_Textures.size()), std::array<GLenum, 6>{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5}.data());
	}


	void GeometryBuffer::OnEventReceived(EventParamType event)
	{
		m_Resolution = event.newResolution;
		SetUpBuffers(m_Resolution);
	}
}
