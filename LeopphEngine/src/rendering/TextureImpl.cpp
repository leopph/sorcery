#include "TextureImpl.hpp"

#include "../util/logger.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <utility>



namespace leopph::impl
{
	TextureImpl::TextureImpl(std::filesystem::path path) :
		Path{std::move(path)},
		Id{m_ID},
		IsTransparent{m_IsTransparent},
		m_ID{},
		m_IsTransparent{}
	{
		stbi_set_flip_vertically_on_load(true);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);

		int width, height, channels;
		const auto data{stbi_load(Path.string().c_str(), &width, &height, &channels, 0)};

		if (data == nullptr)
		{
			glDeleteTextures(1, &m_ID);

			const auto msg{"Texture on path [" + Path.string() + "] could not be loaded."};
			Logger::Instance().Error(msg);
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
				glDeleteTextures(1, &m_ID);

				const auto errMsg{"Texture error: unknown color channel number: [" + std::to_string(channels) + "]."};
				Logger::Instance().Error(errMsg);
				return;
		}

		glTextureStorage2D(m_ID, 1, internalFormat, width, height);
		glTextureSubImage2D(m_ID, 0, 0, 0, width, height, colorFormat, GL_UNSIGNED_BYTE, data);

		glGenerateTextureMipmap(m_ID);

		glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}


	TextureImpl::~TextureImpl()
	{
		glDeleteTextures(1, &Id);
	}
}
