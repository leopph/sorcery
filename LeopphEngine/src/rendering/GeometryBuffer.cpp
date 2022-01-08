#include "GeometryBuffer.hpp"

#include "../windowing/WindowBase.hpp"


namespace leopph::internal
{
	GeometryBuffer::GeometryBuffer() :
		m_Textures{},
		m_FrameBuffer{},
		m_Res{
			[]
			{
				const auto& window{WindowBase::Get()};
				const Vector2 displayRes{window.Width(), window.Height()};
				const auto renderRes{displayRes * window.RenderMultiplier()};
				return ResType{renderRes[0], renderRes[1]};
			}()
		}
	{
		glCreateFramebuffers(1, &m_FrameBuffer);
		InitBuffers();
	}


	GeometryBuffer::~GeometryBuffer() noexcept
	{
		DeinitBuffers();
		glDeleteFramebuffers(1, &m_FrameBuffer);
	}


	auto GeometryBuffer::BindForWritingAndClear() const -> void
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
		glViewport(0, 0, m_Res[0], m_Res[1]);

		glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);
		glClearDepthf(CLEAR_DEPTH);
		glClearStencil(CLEAR_STENCIL);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}


	auto GeometryBuffer::BindForReading(ShaderProgram& shader, const GLuint texUnit) const -> GLuint
	{
		for (auto i{0u}; i < m_Textures.size(); ++i)
		{
			glBindTextureUnit(texUnit + i, m_Textures[i]);
			shader.SetUniform(SHADER_UNIFORM_NAMES[i], static_cast<int>(texUnit + i) /* cast to int because only glUniform1i[v] may be used to set sampler uniforms (wtf?) */);
		}

		return texUnit + static_cast<decltype(texUnit)>(m_Textures.size());
	}


	auto GeometryBuffer::CopyStencilData(const GLuint bufferName) const -> void
	{
		glBlitNamedFramebuffer(m_FrameBuffer, bufferName, 0, 0, static_cast<GLint>(m_Res[0]), static_cast<GLint>(m_Res[1]), 0, 0, static_cast<GLint>(m_Res[0]), static_cast<GLint>(m_Res[1]), GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	}


	auto GeometryBuffer::OnEventReceived(EventParamType event) -> void
	{
		const auto renderRes{event.NewResolution * event.NewResolutionMultiplier};
		m_Res = ResType{renderRes[0], renderRes[1]};
		InitBuffers();
	}


	auto GeometryBuffer::InitBuffers() noexcept -> void
	{
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_Textures.size()), m_Textures.data());

		glTextureStorage2D(m_Textures[NORM_GLOSS_TEX], 1, GL_RGB16F, m_Res[0], m_Res[1]);
		glTextureParameteri(m_Textures[NORM_GLOSS_TEX], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[NORM_GLOSS_TEX], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[DIFF_TEX], 1, GL_RGB8, m_Res[0], m_Res[1]);
		glTextureParameteri(m_Textures[DIFF_TEX], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[DIFF_TEX], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[SPEC_TEX], 1, GL_RGB8, m_Res[0], m_Res[1]);
		glTextureParameteri(m_Textures[SPEC_TEX], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_Textures[SPEC_TEX], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureStorage2D(m_Textures[DEPTH_STENCIL_TEX], 1, GL_DEPTH24_STENCIL8, m_Res[0], m_Res[1]);
		
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT0, m_Textures[NORM_GLOSS_TEX], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT1, m_Textures[DIFF_TEX], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_COLOR_ATTACHMENT2, m_Textures[SPEC_TEX], 0);
		glNamedFramebufferTexture(m_FrameBuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_Textures[DEPTH_STENCIL_TEX], 0);

		const std::array<GLenum, std::tuple_size_v<decltype(m_Textures)>> drawBuffers{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glNamedFramebufferDrawBuffers(m_FrameBuffer, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
	}


	auto GeometryBuffer::DeinitBuffers() const noexcept -> void
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
	}
}
