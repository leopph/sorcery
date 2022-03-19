#include "GlRenderbuffer.hpp"


namespace leopph::internal
{
	GlRenderbuffer::GlRenderbuffer() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateRenderbuffers(1, &name);
				return name;
			}()
		}
	{ }


	GlRenderbuffer::GlRenderbuffer(GlRenderbuffer&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}


	auto GlRenderbuffer::operator=(GlRenderbuffer&& other) noexcept -> GlRenderbuffer&
	{
		glDeleteRenderbuffers(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlRenderbuffer::~GlRenderbuffer() noexcept
	{
		glDeleteRenderbuffers(1, &m_Name);
	}


	GlRenderbuffer::operator GLuint() const noexcept
	{
		return m_Name;
	}


	auto GlRenderbuffer::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
