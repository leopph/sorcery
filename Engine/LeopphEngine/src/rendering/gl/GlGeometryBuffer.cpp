#include "rendering/gl/GlGeometryBuffer.hpp"

#include "InternalContext.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph::internal
{
	GlGeometryBuffer::GlGeometryBuffer() :
		m_Width{
			[]
			{
				auto const* const window = GetWindowImpl();
				return static_cast<int>(window->Width() * window->RenderMultiplier());
			}()
		},
		m_Height{
			[]
			{
				auto const* const window = GetWindowImpl();
				return static_cast<int>(window->Height() * window->RenderMultiplier());
			}()
		}
	{
		ConfigBuffers();
	}


	auto GlGeometryBuffer::Framebuffer() const noexcept -> GlFramebufferObject const&
	{
		return m_Framebuffer;
	}


	auto GlGeometryBuffer::NormalColorGlossTexture() const noexcept -> GlTextureObject<GlTextureType::T2D> const&
	{
		return m_NormClrGlossTexture;
	}


	auto GlGeometryBuffer::DepthStencilTexture() const noexcept -> GlTextureObject<GlTextureType::T2D> const&
	{
		return m_DepthStencilTexture;
	}


	auto GlGeometryBuffer::Width() const noexcept -> int
	{
		return m_Width;
	}


	auto GlGeometryBuffer::Height() const noexcept -> int
	{
		return m_Height;
	}


	auto GlGeometryBuffer::Clear() const -> void
	{
		static GLfloat constexpr clearColor[]{0, 0, 0, 1};
		static GLfloat constexpr clearDepth{1};
		static auto constexpr clearStencil{1};

		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfi(m_Framebuffer, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
	}


	auto GlGeometryBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Width, m_Height);
	}


	auto GlGeometryBuffer::BindForReading(ShaderProgram& shader, GLuint const texUnit) const -> GLuint
	{
		glBindTextureUnit(texUnit, m_NormClrGlossTexture);
		shader.SetUniform("u_NormColorGlossTex", static_cast<int>(texUnit)); // cast to int because only glUniform1i[v] may be used to set sampler uniforms (wtf?)

		glBindTextureUnit(texUnit + 1, m_DepthStencilTexture);
		shader.SetUniform("u_DepthTex", static_cast<int>(texUnit + 1)); // cast to int because only glUniform1i[v] may be used to set sampler uniforms (wtf?)

		return texUnit + 2;
	}


	auto GlGeometryBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Width = static_cast<int>(event.Width * event.RenderMultiplier);
		m_Height = static_cast<int>(event.Height * event.RenderMultiplier);
		ConfigBuffers();
	}


	auto GlGeometryBuffer::ConfigBuffers() noexcept -> void
	{
		m_NormClrGlossTexture = {};
		glTextureStorage2D(m_NormClrGlossTexture, 1, GL_RGB32UI, m_Width, m_Height);
		glTextureParameteri(m_NormClrGlossTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_NormClrGlossTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		m_DepthStencilTexture = {};
		glTextureStorage2D(m_DepthStencilTexture, 1, GL_DEPTH24_STENCIL8, m_Width, m_Height);

		glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_NormClrGlossTexture, 0);
		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthStencilTexture, 0);

		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);
	}
}
