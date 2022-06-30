#include "rendering/opengl/GlBuffer.hpp"


namespace leopph::internal
{
	GlBuffer::GlBuffer() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateBuffers(1, &name);
				return name;
			}()
		}
	{ }


	GlBuffer::GlBuffer(GlBuffer&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}


	auto GlBuffer::operator=(GlBuffer&& other) noexcept -> GlBuffer&
	{
		glDeleteBuffers(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlBuffer::~GlBuffer() noexcept
	{
		glDeleteBuffers(1, &m_Name);
	}


	GlBuffer::operator GLuint() const noexcept
	{
		return m_Name;
	}


	auto GlBuffer::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
