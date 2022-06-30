#include "rendering/gl/GlTransparencyBuffer.hpp"

#include "InternalContext.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph::internal
{
	GlTransparencyBuffer::GlTransparencyBuffer(GlRenderbufferObject const* depthBuffer) :
		m_DepthBuffer{depthBuffer},
		m_Width{
			[]
			{
				auto const& window = GetWindowImpl();
				return static_cast<int>(window->Width() * window->RenderMultiplier());
			}()
		},
		m_Height{
			[]
			{
				auto const& window = GetWindowImpl();
				return static_cast<int>(window->Height() * window->RenderMultiplier());
			}()
		}
	{
		ConfigBuffers();
	}


	auto GlTransparencyBuffer::Clear() const -> void
	{
		GLfloat constexpr zero4[]{0, 0, 0, 0};
		GLfloat constexpr one4[]{1, 0, 0, 0};
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, zero4);
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 1, one4);
	}


	auto GlTransparencyBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *m_DepthBuffer);
		glViewport(0, 0, m_Width, m_Height);
	}


	auto GlTransparencyBuffer::BindForReading(ShaderProgram& shader, GLint const nextFreeTexUnit) const -> GLint
	{
		glBindTextureUnit(nextFreeTexUnit, m_AccumBuffer);
		glBindTextureUnit(nextFreeTexUnit + 1, m_RevealBuffer);
		shader.SetUniform("u_AccumTex", nextFreeTexUnit);
		shader.SetUniform("u_RevealTex", nextFreeTexUnit + 1);
		return nextFreeTexUnit + 1;
	}


	auto GlTransparencyBuffer::ConfigBuffers() -> void
	{
		m_AccumBuffer = {};
		glTextureStorage2D(m_AccumBuffer, 1, GL_RGBA16F, m_Width, m_Height);

		m_RevealBuffer = {};
		glTextureStorage2D(m_RevealBuffer, 1, GL_R8, m_Width, m_Height);

		glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_AccumBuffer, 0);
		glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT1, m_RevealBuffer, 0);

		GLenum constexpr bufTargets[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glNamedFramebufferDrawBuffers(m_Framebuffer, 2, bufTargets);
	}


	auto GlTransparencyBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Width = static_cast<int>(event.Width * event.RenderMultiplier);
		m_Height = static_cast<int>(event.Height * event.RenderMultiplier);
		ConfigBuffers();
	}
}
