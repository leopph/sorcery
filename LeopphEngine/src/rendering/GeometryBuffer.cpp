#include "GeometryBuffer.hpp"

#include "../windowing/WindowBase.hpp"

#include <glad/glad.h>

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
			glClearNamedFramebufferfv(m_FrameBuffer, GL_COLOR, static_cast<GLint>(i), Vector4{static_cast<Vector3>(WindowBase::Get().ClearColor())}.Data().data());
		}

		glClearNamedFramebufferfv(m_FrameBuffer, GL_DEPTH, 0, std::array<GLfloat, 1>{1}.data());
	}

	auto GeometryBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
		glViewport(0, 0, static_cast<GLsizei>(m_Resolution[0]), static_cast<GLsizei>(m_Resolution[1]));
	}

	auto GeometryBuffer::UnbindFromWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const auto& window{WindowBase::Get()};
		glViewport(0, 0, static_cast<GLsizei>(window.Width()), static_cast<GLsizei>(window.Height()));
	}

	auto GeometryBuffer::BindForReading(ShaderProgram& shader, const TextureType type, int texUnit) const -> int
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

	auto GeometryBuffer::BindForReading(ShaderProgram& shader, int texUnit) const -> int
	{
		for (std::size_t i = 0; i < m_Textures.size(); ++i)
		{
			texUnit = BindForReading(shader, static_cast<TextureType>(i), texUnit);
		}

		return texUnit;
	}

	auto GeometryBuffer::UnbindFromReading(const TextureType type) const -> void
	{
		if (m_BindIndices[type] != BIND_FILL_VALUE)
		{
			glBindTextureUnit(m_BindIndices[type], 0);
			m_BindIndices[type] = BIND_FILL_VALUE;
		}
	}

	auto GeometryBuffer::UnbindFromReading() const -> void
	{
		for (std::size_t i = 0; i < m_Textures.size(); ++i)
		{
			UnbindFromReading(static_cast<TextureType>(i));
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

	auto GeometryBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Resolution = event.NewResolution * event.NewResolutionMultiplier;
		SetUpBuffers(m_Resolution);
	}
}
