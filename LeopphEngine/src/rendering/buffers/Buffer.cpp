#include "Buffer.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	Buffer::Buffer() :
		name{ m_Name }, m_Name{}
	{
		glGenBuffers(1, &m_Name);
	}


	Buffer::~Buffer()
	{
		glDeleteBuffers(1, &m_Name);
	}
}