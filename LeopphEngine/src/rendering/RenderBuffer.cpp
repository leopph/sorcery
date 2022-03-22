#include "RenderBuffer.hpp"

#include "../windowing/WindowImpl.hpp"


namespace leopph::internal
{
	RenderBuffer::RenderBuffer() :
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


	auto RenderBuffer::Clear() const -> void
	{
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, static_cast<WindowImpl*>(Window::Instance())->ClearColor().Data().data());
		glClearNamedFramebufferfi(m_Framebuffer, GL_DEPTH_STENCIL, 0, CLEAR_DEPTH, CLEAR_STENCIL);
	}


	auto RenderBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Width, m_Height);
	}


	auto RenderBuffer::Framebuffer() const noexcept -> GlFramebuffer const&
	{
		return m_Framebuffer;
	}


	auto RenderBuffer::ColorBuffer() const noexcept -> GlRenderbuffer const&
	{
		return m_ColorBuffer;
	}


	auto RenderBuffer::DepthStencilBuffer() const noexcept -> GlRenderbuffer const&
	{
		return m_DepthStencilBuffer;
	}


	auto RenderBuffer::Width() const noexcept -> int
	{
		return m_Width;
	}


	auto RenderBuffer::Height() const noexcept -> int
	{
		return m_Height;
	}


	auto RenderBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Width = static_cast<float>(event.Width) * event.RenderMultiplier;
		m_Height = static_cast<float>(event.Height) * event.RenderMultiplier;
		InitBuffers();
	}


	auto RenderBuffer::InitBuffers() noexcept -> void
	{
		m_ColorBuffer = GlRenderbuffer{};
		glNamedRenderbufferStorage(m_ColorBuffer, GL_RGBA8, m_Width, m_Height);

		m_DepthStencilBuffer = GlRenderbuffer{};
		glNamedRenderbufferStorage(m_DepthStencilBuffer, GL_DEPTH24_STENCIL8, m_Width, m_Height);

		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBuffer);
		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilBuffer);

		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);
	}
}
