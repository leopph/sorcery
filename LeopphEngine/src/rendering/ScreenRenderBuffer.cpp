#include "ScreenRenderBuffer.hpp"

#include "../windowing/WindowImpl.hpp"


namespace leopph::internal
{
	ScreenRenderBuffer::ScreenRenderBuffer() :
		m_Width{
			[]
			{
				auto const& window{Window::Instance()};
				return static_cast<int>(window->Width() * window->RenderMultiplier());
			}()
		},
		m_Height{
			[]
			{
				auto const& window{Window::Instance()};
				return static_cast<int>(window->Height() * window->RenderMultiplier());
			}
			()
		}
	{
		InitBuffers();
	}


	auto ScreenRenderBuffer::Clear() const -> void
	{
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, static_cast<WindowImpl*>(Window::Instance())->ClearColor().Data().data());
		glClearNamedFramebufferfi(m_Framebuffer, GL_DEPTH_STENCIL, 0, CLEAR_DEPTH, CLEAR_STENCIL);
	}


	auto ScreenRenderBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Width, m_Height);
	}


	auto ScreenRenderBuffer::CopyColorToDefaultFramebuffer() const -> void
	{
		auto const& window{Window::Instance()};
		glBlitNamedFramebuffer(m_Framebuffer, 0, 0, 0, m_Width, m_Height, 0, 0, static_cast<GLsizei>(window->Width()), static_cast<GLsizei>(window->Height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}


	auto ScreenRenderBuffer::FramebufferName() const noexcept -> GLuint
	{
		return m_Framebuffer;
	}


	auto ScreenRenderBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Width = static_cast<float>(event.Width) * event.RenderMultiplier;
		m_Height  = static_cast<float>(event.Height) * event.RenderMultiplier;
		InitBuffers();
	}


	auto ScreenRenderBuffer::InitBuffers() noexcept -> void
	{
		m_ColorBuffer = GlRenderbuffer{};
		glNamedRenderbufferStorage(m_ColorBuffer, GL_RGB8, m_Width, m_Height);

		m_DepthStencilBuffer = GlRenderbuffer{};
		glNamedRenderbufferStorage(m_DepthStencilBuffer, GL_DEPTH24_STENCIL8, m_Width, m_Height);

		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBuffer);
		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilBuffer);

		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);
	}
}
