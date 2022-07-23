#include "GlVertexArrayObject.hpp"


namespace leopph::internal
{
	GlVertexArrayObject::GlVertexArrayObject() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateVertexArrays(1, &name);
				return name;
			}()
		}
	{ }



	GlVertexArrayObject::GlVertexArrayObject(GlVertexArrayObject&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}



	GlVertexArrayObject& GlVertexArrayObject::operator=(GlVertexArrayObject&& other) noexcept
	{
		glDeleteVertexArrays(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}



	GlVertexArrayObject::~GlVertexArrayObject() noexcept
	{
		glDeleteVertexArrays(1, &m_Name);
	}



	GlVertexArrayObject::operator GLuint() const noexcept
	{
		return m_Name;
	}



	GLuint GlVertexArrayObject::Name() const noexcept
	{
		return m_Name;
	}
}
