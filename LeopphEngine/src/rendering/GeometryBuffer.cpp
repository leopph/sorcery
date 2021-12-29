#include "GeometryBuffer.hpp"

#include "../windowing/WindowBase.hpp"

#include <algorithm>
#include <cstddef>


namespace leopph::internal
{
	GeometryBuffer::GeometryBuffer() :
		m_Textures{},
		m_BindIndices{},
		m_DepthBuffer{},
		m_FrameBuffer{},
		m_Resolution{Vector2{WindowBase::Get().Width(), WindowBase::Get().Height()} * WindowBase::Get().RenderMultiplier()}
	{
		std::ranges::fill(m_BindIndices, BIND_FILL_VALUE);
		glCreateFramebuffers(1, &m_FrameBuffer);
		SetUpBuffers(m_Resolution);
	}


	GeometryBuffer::~GeometryBuffer()
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
		glDeleteRenderbuffers(1, &m_DepthBuffer);
		glDeleteFramebuffers(1, &m_FrameBuffer);
	}


	auto GeometryBuffer::Clear() const -> void
	{
		for (std::size_t i = 0; i < m_Textures.size(); i++)
		{
			glClearNamedFramebufferfv(m_FrameBuffer, GL_COLOR, static_cast<GLint>(i), CLEAR_COLOR);
		}
		glClearNamedFramebufferfi(m_FrameBuffer, GL_DEPTH_STENCIL, 0, CLEAR_DEPTH, CLEAR_STENCIL);
	}


	auto GeometryBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
		glViewport(0, 0, static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]));
	}


	auto GeometryBuffer::UnbindFromWriting() -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const auto& window{WindowBase::Get()};
		glViewport(0, 0, static_cast<GLsizei>(window.Width()), static_cast<GLsizei>(window.Height()));
	}


	auto GeometryBuffer::BindForReading(ShaderProgram& shader, const Texture type, int texUnit) const -> int
	{
		glBindTextureUnit(static_cast<unsigned>(texUnit), m_Textures[static_cast<int>(type)]);

		auto uniformName{""};

		switch (type)
		{
		case Texture::Position:
				uniformName = "u_PositionTexture";
				break;

		case Texture::Normal:
				uniformName = "u_NormalTexture";
				break;

		case Texture::Ambient:
				uniformName = "u_AmbientTexture";
				break;

		case Texture::Diffuse:
				uniformName = "u_DiffuseTexture";
				break;

		case Texture::Specular:
				uniformName = "u_SpecularTexture";
				break;

		case Texture::Shine:
				uniformName = "u_ShineTexture";
				break;
		}

		shader.SetUniform(uniformName, texUnit);

		m_BindIndices[static_cast<int>(type)] = texUnit;
		return ++texUnit;
	}


	auto GeometryBuffer::BindForReading(ShaderProgram& shader, int texUnit) const -> int
	{
		for (std::size_t i = 0; i < m_Textures.size(); ++i)
		{
			texUnit = BindForReading(shader, static_cast<Texture>(i), texUnit);
		}

		return texUnit;
	}


	auto GeometryBuffer::UnbindFromReading(const Texture type) const -> void
	{
		if (m_BindIndices[static_cast<int>(type)] != BIND_FILL_VALUE)
		{
			glBindTextureUnit(m_BindIndices[static_cast<int>(type)], 0);
			m_BindIndices[static_cast<int>(type)] = BIND_FILL_VALUE;
		}
	}


	auto GeometryBuffer::UnbindFromReading() const -> void
	{
		for (std::size_t i = 0; i < m_Textures.size(); ++i)
		{
			UnbindFromReading(static_cast<Texture>(i));
		}
	}


	auto GeometryBuffer::CopyDepthData(const unsigned bufferName) const -> void
	{
		glBlitNamedFramebuffer(m_FrameBuffer, bufferName, 0, 0, static_cast<GLint>(m_Resolution[0]), static_cast<GLint>(m_Resolution[1]), 0, 0, static_cast<GLint>(m_Resolution[0]), static_cast<GLint>(m_Resolution[1]), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}


	auto GeometryBuffer::SetUpBuffers(const Vector2& res) -> void
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
		glDeleteRenderbuffers(1, &m_DepthBuffer);

		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_Textures.size()), m_Textures.data());

		glTextureStorage2D(m_Textures[static_cast<int>(Texture::Position)], 1, GL_RGBA32F, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Position)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Position)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[static_cast<int>(Texture::Normal)], 1, GL_RGB32F, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Normal)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Normal)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[static_cast<int>(Texture::Ambient)], 1, GL_RGB8, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Ambient)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Ambient)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[static_cast<int>(Texture::Diffuse)], 1, GL_RGB8, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Diffuse)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Diffuse)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[static_cast<int>(Texture::Specular)], 1, GL_RGB8, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Specular)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Specular)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[static_cast<int>(Texture::Shine)], 1, GL_R32F, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Shine)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[static_cast<int>(Texture::Shine)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glCreateRenderbuffers(1, &m_DepthBuffer);
		glNamedRenderbufferStorage(m_DepthBuffer, GL_DEPTH24_STENCIL8, static_cast<GLint>(res[0]), static_cast<GLint>(res[1]));

		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT0, m_Textures[static_cast<int>(Texture::Position)], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT1, m_Textures[static_cast<int>(Texture::Normal)], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT2, m_Textures[static_cast<int>(Texture::Ambient)], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT3, m_Textures[static_cast<int>(Texture::Diffuse)], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT4, m_Textures[static_cast<int>(Texture::Specular)], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT5, m_Textures[static_cast<int>(Texture::Shine)], 0);
		glNamedFramebufferRenderbuffer(m_FrameBuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer);

		glNamedFramebufferDrawBuffers(m_FrameBuffer, static_cast<GLsizei>(m_Textures.size()), std::array<GLenum, 6>{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5}.data());
	}


	auto GeometryBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Resolution = event.NewResolution * event.NewResolutionMultiplier;
		SetUpBuffers(m_Resolution);
	}
}
