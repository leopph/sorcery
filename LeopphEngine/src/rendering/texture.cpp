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
		m_Path{std::move(path)}
	{
		stbi_set_flip_vertically_on_load(true);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TexName);

		int width, height, channels;
		const auto data{stbi_load(m_Path.string().c_str(), &width, &height, &channels, 0)};

		if (data == nullptr)
		{
			glDeleteTextures(1, &m_TexName);

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
				glDeleteTextures(1, &m_TexName);

				const auto errMsg{"Texture error: unknown color channel number: [" + std::to_string(channels) + "]."};
				internal::Logger::Instance().Error(errMsg);
				return;
		}

		glTextureStorage2D(m_TexName, 1, internalFormat, width, height);
		glTextureSubImage2D(m_TexName, 0, 0, 0, width, height, colorFormat, GL_UNSIGNED_BYTE, data);

		glGenerateTextureMipmap(m_TexName);

		glTextureParameteri(m_TexName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_TexName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		internal::DataManager::Instance().RegisterTexture(this);
	}


	Texture::~Texture()
	{
		glDeleteTextures(1, &m_TexName);
		internal::DataManager::Instance().UnregisterTexture(this);
	}
}
