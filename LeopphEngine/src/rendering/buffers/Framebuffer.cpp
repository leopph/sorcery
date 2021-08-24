#include "Framebuffer.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	Framebuffer::Framebuffer() :
		name{ m_Name }, m_Name{}
	{
		glCreateFramebuffers(1, &m_Name);
	}


	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_Name);
	}
}