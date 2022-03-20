#include "Texture.hpp"

#include "../data/DataManager.hpp"
#include "../util/Logger.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <stb_image.h>
#include <utility>


namespace leopph
{
	Texture::Texture(std::filesystem::path path) :
		m_Texture{},
		m_Path{std::move(path)},
		m_SemiTransparent{},
		m_Transparent{},
		m_Width{},
		m_Height{}
	{
		internal::DataManager::Instance().RegisterTexture(this);

		stbi_set_flip_vertically_on_load(true);
		int channels;
		auto const data{stbi_load(m_Path.string().c_str(), &m_Width, &m_Height, &channels, 0)};

		if (data == nullptr)
		{
			auto const msg{"Failed to load texture at " + m_Path.string() + "."};
			internal::Logger::Instance().Error(msg);
			return;
		}

		GLenum colorFormat;
		GLenum internalFormat;

		switch (channels)
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
				std::span const dataSpan{data, static_cast<std::size_t>(m_Width * m_Height * 4)};
				m_SemiTransparent = CheckSemiTransparency(dataSpan);

				if (m_SemiTransparent)
				{
					m_Transparent = CheckFullTransparency(dataSpan);
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
			{
				auto const errMsg{"Unhandled number of color channels in texture at " + m_Path.string() + ": " + std::to_string(channels) + "."};
				internal::Logger::Instance().Error(errMsg);
				stbi_image_free(data);
				return;
			}
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_Texture);
		glTextureStorage2D(m_Texture, 1, internalFormat, m_Width, m_Height);
		glTextureSubImage2D(m_Texture, 0, 0, 0, m_Width, m_Height, colorFormat, GL_UNSIGNED_BYTE, data);

		glGenerateTextureMipmap(m_Texture);

		glTextureParameteri(m_Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}


	Texture::~Texture() noexcept
	{
		glDeleteTextures(1, &m_Texture);
		internal::DataManager::Instance().UnregisterTexture(this);
	}


	auto Texture::operator<=>(Texture const& other) const -> std::strong_ordering
	{
		return m_Path <=> other.m_Path;
	}


	auto Texture::operator==(Texture const& other) const -> bool
	{
		return m_Path == other.m_Path;
	}


	auto Texture::IsSemiTransparent() const -> bool
	{
		return m_SemiTransparent;
	}


	auto Texture::IsTransparent() const -> bool
	{
		return m_Transparent;
	}


	auto Texture::TextureName() const -> GLuint
	{
		return m_Texture;
	}


	auto Texture::Path() const -> std::filesystem::path const&
	{
		return m_Path;
	}


	auto Texture::Width() const noexcept -> int
	{
		return m_Width;
	}


	auto Texture::Height() const noexcept -> int
	{
		return m_Height;
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


	auto operator<=>(std::filesystem::path const& path, Texture const& tex) -> std::strong_ordering
	{
		return path <=> tex.Path();
	}


	auto operator<=>(Texture const& tex, std::filesystem::path const& path) -> std::strong_ordering
	{
		return tex.Path() <=> path;
	}


	auto operator==(Texture const& tex, std::filesystem::path const& path) -> bool
	{
		return tex.Path() == path;
	}


	auto operator==(std::filesystem::path const& path, Texture const& tex) -> bool
	{
		return path == tex.Path();
	}
}
