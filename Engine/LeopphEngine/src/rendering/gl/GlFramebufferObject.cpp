#include "rendering/gl/GlFramebufferObject.hpp"


namespace leopph::internal
{
	GlFramebufferObject::GlFramebufferObject() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateFramebuffers(1, &name);
				return name;
			}()
		}
	{ }


	GlFramebufferObject::GlFramebufferObject(GlFramebufferObject&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}


	auto GlFramebufferObject::operator=(GlFramebufferObject&& other) noexcept -> GlFramebufferObject&
	{
		glDeleteFramebuffers(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlFramebufferObject::~GlFramebufferObject() noexcept
	{
		glDeleteFramebuffers(1, &m_Name);
	}


	GlFramebufferObject::operator GLuint() const noexcept
	{
		return m_Name;
	}


	auto GlFramebufferObject::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
