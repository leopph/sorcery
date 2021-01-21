#include "texture.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <stdexcept>

namespace leopph
{
	// INIT REF COUNTER
	std::unordered_map<unsigned, size_t> Texture::s_Instances{};


	// LOAD IMAGE FROM PATH
	Texture::Texture(const std::filesystem::path& path, TextureType type)
		: m_Path{ path }, m_Type{ type }
	{
		glGenTextures(1, &m_ID);

		// load image
		int width, height, channels;
		unsigned char* data{ stbi_load(path.string().c_str(), &width, &height, &channels, 0) };

		if (data == nullptr)
			throw std::exception{};

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
			throw std::exception{};
		}

		// load texture data
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, width, height, 0, colorFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		stbi_image_free(data);

		s_Instances.try_emplace(m_ID, 0);
		s_Instances[m_ID]++;
	}


	Texture::~Texture()
	{
		s_Instances[m_ID]--;

		if (s_Instances[m_ID] == 0)
			glDeleteTextures(1, &m_ID);
	}


	Texture::Texture(const Texture& other)
		: m_ID{ other.m_ID }, m_Path{ other.m_Path }, m_Type{ other.m_Type }
	{
		s_Instances[m_ID]++;
	}


	Texture::Texture(Texture&& other) noexcept
		: m_ID{ other.m_ID }, m_Path{ std::move(other.m_Path) }, m_Type{ other.m_Type }
	{
		other.m_ID = 0;
		other.m_Path.clear();

		s_Instances.try_emplace(0, 0);
		s_Instances[0]++;
	}


	Texture& Texture::operator=(const Texture& other)
	{
		if (*this == other)
			return *this;

		s_Instances[m_ID]--;

		if (s_Instances[m_ID] == 0)
			glDeleteTextures(1, &m_ID);

		this->m_ID = other.m_ID;
		this->m_Path = other.m_Path;
		this->m_Type = other.m_Type;

		s_Instances[m_ID]++;

		return *this;
	}


	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (*this == other)
			return *this;

		s_Instances[m_ID]--;

		if (s_Instances[m_ID] == 0)
			glDeleteTextures(1, &m_ID);

		this->m_ID = other.m_ID;
		this->m_Path = std::move(other.m_Path);
		this->m_Type = other.m_Type;

		other.m_ID = 0;
		other.m_Path.clear();

		s_Instances.try_emplace(0, 0);
		s_Instances[0]++;

		return *this;
	}


	bool Texture::operator==(const Texture& other) const
	{
		return this->m_ID == other.m_ID;
	}


	bool Texture::operator==(const std::filesystem::path& path) const
	{
		return this->m_Path == path;
	}

	
	unsigned Texture::ID() const
	{
		return m_ID;
	}

	
	Texture::TextureType Texture::Type() const
	{
		return m_Type;
	}
}