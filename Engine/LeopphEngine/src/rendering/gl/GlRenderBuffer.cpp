#include "rendering/gl/GlRenderBuffer.hpp"

#include "InternalContext.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph::internal
{
	GlRenderBuffer::GlRenderBuffer() :
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
			}
			()
		}
	{
		InitBuffers();
	}


	auto GlRenderBuffer::Clear() const -> void
	{
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, GetWindowImpl()->ClearColor().Data().data());
		glClearNamedFramebufferfi(m_Framebuffer, GL_DEPTH_STENCIL, 0, CLEAR_DEPTH, CLEAR_STENCIL);
	}


	auto GlRenderBuffer::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Width, m_Height);
	}


	auto GlRenderBuffer::Framebuffer() const noexcept -> GlFramebufferObject const&
	{
		return m_Framebuffer;
	}


	auto GlRenderBuffer::ColorBuffer() const noexcept -> GlRenderbufferObject const&
	{
		return m_ColorBuffer;
	}


	auto GlRenderBuffer::DepthStencilBuffer() const noexcept -> GlRenderbufferObject const&
	{
		return m_DepthStencilBuffer;
	}


	auto GlRenderBuffer::Width() const noexcept -> int
	{
		return m_Width;
	}


	auto GlRenderBuffer::Height() const noexcept -> int
	{
		return m_Height;
	}


	auto GlRenderBuffer::OnEventReceived(EventParamType event) -> void
	{
		m_Width = static_cast<int>(event.Width * event.RenderMultiplier);
		m_Height = static_cast<int>(event.Height * event.RenderMultiplier);
		InitBuffers();
	}


	auto GlRenderBuffer::InitBuffers() noexcept -> void
	{
		m_ColorBuffer = GlRenderbufferObject{};
		glNamedRenderbufferStorage(m_ColorBuffer, GL_RGBA8, m_Width, m_Height);

		m_DepthStencilBuffer = GlRenderbufferObject{};
		glNamedRenderbufferStorage(m_DepthStencilBuffer, GL_DEPTH24_STENCIL8, m_Width, m_Height);

		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBuffer);
		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilBuffer);

		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);
	}
}
