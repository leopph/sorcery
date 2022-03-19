#include "GlTexture2D.hpp"


namespace leopph::internal
{
	GlTexture2D::GlTexture2D() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateTextures(GL_TEXTURE_2D, 1, &name);
				return name;
			}()
		}
	{ }


	GlTexture2D::GlTexture2D(GlTexture2D&& other) noexcept :
		m_Name{other.Name()}
	{
		other.m_Name = 0;
	}


	auto GlTexture2D::operator=(GlTexture2D&& other) noexcept -> GlTexture2D&
	{
		glDeleteTextures(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	GlTexture2D::~GlTexture2D() noexcept
	{
		glDeleteTextures(1, &m_Name);
	}


	GlTexture2D::operator GLuint() const noexcept
	{
		return m_Name;
	}


	auto GlTexture2D::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
