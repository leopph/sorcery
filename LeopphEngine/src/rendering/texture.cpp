#include "texture.h"

#include "../instances/instanceholder.h"
#include "../util/logger.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <stdexcept>
#include <string>

namespace leopph::impl
{
	Texture::Texture(const std::filesystem::path& path)
		: path{ path }, id{ m_ID }
	{
		stbi_set_flip_vertically_on_load(true);

		glGenTextures(1, &m_ID);

		int width, height, channels;
		unsigned char* data{ stbi_load(path.string().c_str(), &width, &height, &channels, 0) };

		if (data == nullptr)
		{
			const auto msg{ "Texture on path [" + path.string() + "] could not be loaded." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		GLenum colorFormat{};

		switch (channels)
		{
		case 1:
			colorFormat = GL_RED;
			break;

		case 3:
			colorFormat = GL_RGB;
			break;

		case 4:
			colorFormat = GL_RGBA;
			break;

		default:
			stbi_image_free(data);
			const auto msg{ "Unknown channel number [" + std::to_string(channels) + "]."};
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, width, height, 0, colorFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		stbi_image_free(data);

		InstanceHolder::StoreTexture(*this);
	}


	Texture::~Texture()
	{
		InstanceHolder::RemoveTexture(path);
		if (!InstanceHolder::IsTextureStored(path))
			glDeleteTextures(1, &id);
	}


	Texture::Texture(const Texture& other)
		: m_ID{ other.m_ID }, path{ other.path }, id{ m_ID }
	{
		InstanceHolder::AddTexture(path);
	}

	Texture::Texture(const TextureReference& reference) :
		m_ID{ reference.id }, id{ m_ID }, path{ reference.path }
	{
		InstanceHolder::AddTexture(path);
	}

	bool Texture::operator==(const Texture& other) const
	{
		return this->id == other.id;
	}


	bool Texture::operator==(const std::filesystem::path& other) const
	{
		return this->path == other;
	}
}