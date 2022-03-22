#include "Texture.hpp"

#include "../data/DataManager.hpp"
#include "../util/Logger.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <string>


namespace leopph
{
	Texture::Texture(Image const& img) :
		m_Texture{},
		m_SemiTransparent{},
		m_Transparent{},
		m_Width{img.Width()},
		m_Height{img.Height()}
	{
		GLenum colorFormat;
		GLenum internalFormat;

		switch (img.Channels())
		{
			case 1:
			{
				colorFormat = GL_RED;
				internalFormat = GL_R8;
				break;
			}

			case 3:
			{
				colorFormat = GL_RGB;
				internalFormat = GL_RGB8;
				break;
			}

			case 4:
			{
				m_SemiTransparent = CheckSemiTransparency(img.Data());

				if (m_SemiTransparent)
				{
					m_Transparent = CheckFullTransparency(img.Data());
					colorFormat = GL_RGBA;
					internalFormat = GL_RGBA8;
				}
				else
				{
					m_Transparent = false;
					colorFormat = GL_RGB;
					internalFormat = GL_RGB8;
				}
				break;
			}

			default:
				internal::Logger::Instance().Error("Invalid image channel count \"" + std::to_string(img.Channels()) + "\" while loading texture.");
				return;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_Texture);
		glTextureStorage2D(m_Texture, 1, internalFormat, m_Width, m_Height);
		glTextureSubImage2D(m_Texture, 0, 0, 0, m_Width, m_Height, colorFormat, GL_UNSIGNED_BYTE, img.Data().data());

		glGenerateTextureMipmap(m_Texture);

		glTextureParameteri(m_Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}


	Texture::~Texture() noexcept
	{
		glDeleteTextures(1, &m_Texture);
	}


	auto Texture::TextureName() const -> GLuint
	{
		return m_Texture;
	}


	auto Texture::Width() const noexcept -> int
	{
		return m_Width;
	}


	auto Texture::Height() const noexcept -> int
	{
		return m_Height;
	}


	auto Texture::IsSemiTransparent() const -> bool
	{
		return m_SemiTransparent;
	}


	auto Texture::IsTransparent() const -> bool
	{
		return m_Transparent;
	}


	auto Texture::CheckSemiTransparency(std::span<unsigned char const> const data) -> bool
	{
		for (std::size_t i = 3; i < data.size(); i += 4)
		{
			if (data[i] != 255)
			{
				return true;
			}
		}

		return false;
	}


	auto Texture::CheckFullTransparency(std::span<unsigned char const> const data) -> bool
	{
		for (std::size_t i = 3; i < data.size(); i += 4)
		{
			if (data[i] == 255)
			{
				return false;
			}
		}

		return true;
	}
}
