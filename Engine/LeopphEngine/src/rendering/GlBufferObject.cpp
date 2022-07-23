#include "GlBufferObject.hpp"


namespace leopph::internal
{
	GlBufferObject::GlBufferObject() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateBuffers(1, &name);
				return name;
			}()
		}
	{ }


	GlBufferObject::GlBufferObject(GlBufferObject&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}


	GlBufferObject& GlBufferObject::operator=(GlBufferObject&& other) noexcept
	{
		glDeleteBuffers(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlBufferObject::~GlBufferObject() noexcept
	{
		glDeleteBuffers(1, &m_Name);
	}


	GlBufferObject::operator GLuint() const noexcept
	{
		return m_Name;
	}


	GLuint GlBufferObject::Name() const noexcept
	{
		return m_Name;
	}
}
