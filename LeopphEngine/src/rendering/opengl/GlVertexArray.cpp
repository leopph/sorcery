#include "GlVertexArray.hpp"


namespace leopph::internal
{
	GlVertexArray::GlVertexArray() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateVertexArrays(1, &name);
				return name;
			}()
		}
	{ }


	GlVertexArray::GlVertexArray(GlVertexArray&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}


	auto GlVertexArray::operator=(GlVertexArray&& other) noexcept -> GlVertexArray&
	{
		glDeleteVertexArrays(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlVertexArray::~GlVertexArray() noexcept
	{
		glDeleteVertexArrays(1, &m_Name);
	}


	GlVertexArray::operator GLuint() const noexcept
	{
		return m_Name;
	}


	auto GlVertexArray::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
