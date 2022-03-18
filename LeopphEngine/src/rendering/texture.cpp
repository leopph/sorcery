#include "Texture.hpp"

#include "../data/DataManager.hpp"
#include "../util/Logger.hpp"

#include <glad/gl.h>

#include <stb_image.h>
#include <utility>


namespace leopph
{
	Texture::Texture(std::filesystem::path path) :
		m_TexName{},
		m_IsTransparent{},
		m_Path{std::move(path)},
		m_Width{},
		m_Height{}
	{
		internal::DataManager::Instance().RegisterTexture(this);

		stbi_set_flip_vertically_on_load(true);
		int channels;
		const auto data{stbi_load(m_Path.string().c_str(), &m_Width, &m_Height, &channels, 0)};

		if (data == nullptr)
		{
			const auto msg{"Texture on path [" + m_Path.string() + "] could not be loaded."};
			internal::Logger::Instance().Error(msg);
			return;
		}

		GLenum colorFormat;
		GLenum internalFormat;

		switch (channels)
		{
			case 1:
				colorFormat = GL_RED;
				internalFormat = GL_R8;
				break;

			case 3:
				colorFormat = GL_RGB;
				internalFormat = GL_RGB8;
				break;

			case 4:
				colorFormat = GL_RGBA;
				internalFormat = GL_RGBA8;
				m_IsTransparent = true;
				break;

			default:
				stbi_image_free(data);
				const auto errMsg{"Texture error: unknown color channel number: [" + std::to_string(channels) + "]."};
				internal::Logger::Instance().Error(errMsg);
				return;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TexName);
		glTextureStorage2D(m_TexName, 1, internalFormat, m_Width, m_Height);
		glTextureSubImage2D(m_TexName, 0, 0, 0, m_Width, m_Height, colorFormat, GL_UNSIGNED_BYTE, data);

		glGenerateTextureMipmap(m_TexName);

		glTextureParameteri(m_TexName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_TexName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}


	Texture::~Texture() noexcept
	{
		glDeleteTextures(1, &m_TexName);
		internal::DataManager::Instance().UnregisterTexture(this);
	}
}
