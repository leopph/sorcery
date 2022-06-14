#include "GlFramebuffer.hpp"


namespace leopph::internal
{
	GlFramebuffer::GlFramebuffer() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateFramebuffers(1, &name);
				return name;
			}()
		}
	{ }


	GlFramebuffer::GlFramebuffer(GlFramebuffer&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}


	auto GlFramebuffer::operator=(GlFramebuffer&& other) noexcept -> GlFramebuffer&
	{
		glDeleteFramebuffers(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlFramebuffer::~GlFramebuffer() noexcept
	{
		glDeleteFramebuffers(1, &m_Name);
	}


	GlFramebuffer::operator GLuint() const noexcept
	{
		return m_Name;
	}


	auto GlFramebuffer::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
