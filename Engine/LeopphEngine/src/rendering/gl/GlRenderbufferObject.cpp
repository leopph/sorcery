#include "rendering/gl/GlRenderbufferObject.hpp"


namespace leopph::internal
{
	GlRenderbufferObject::GlRenderbufferObject() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateRenderbuffers(1, &name);
				return name;
			}()
		}
	{ }


	GlRenderbufferObject::GlRenderbufferObject(GlRenderbufferObject&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}


	auto GlRenderbufferObject::operator=(GlRenderbufferObject&& other) noexcept -> GlRenderbufferObject&
	{
		glDeleteRenderbuffers(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlRenderbufferObject::~GlRenderbufferObject() noexcept
	{
		glDeleteRenderbuffers(1, &m_Name);
	}


	GlRenderbufferObject::operator GLuint() const noexcept
	{
		return m_Name;
	}


	auto GlRenderbufferObject::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
