#include "Image.hpp"

#include "../util/Logger.hpp"

#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <utility>


namespace leopph
{
	Image::Image(std::filesystem::path src, bool const flipVertically) :
		m_Path{std::move(src)}
	{
		stbi_set_flip_vertically_on_load(flipVertically);
		auto const pathStr{m_Path.string()};
		auto const data = stbi_load(pathStr.c_str(), &m_Width, &m_Height, &m_Channels, 0);

		if (!data)
		{
			internal::Logger::Instance().Error("Failed to load image at " + pathStr + ". Reverting to default image.");
			m_Path.clear();
			m_Width = 0;
			m_Height = 0;
			m_Channels = 0;
			return;
		}

		m_Bytes.assign(data, data + m_Width * m_Height * m_Channels);

		stbi_image_free(data);
	}


	Image::Image(int const width, int const height, int const channels, std::vector<unsigned char> bytes) :
		m_Width{width},
		m_Height{height},
		m_Channels{channels},
		m_Bytes{std::move(bytes)}
	{
		if (m_Bytes.size() != m_Width * m_Height * m_Channels)
		{
			internal::Logger::Instance().Error("Inconsistent arguments detected. The number of bytes passed is not equal to the given image parameters. Expected byte count was " + std::to_string(m_Width * m_Height * m_Channels) + " but " + std::to_string(m_Bytes.size()) + " was given. Reverting to default image.");
			m_Width = 0;
			m_Height = 0;
			m_Channels = 0;
			m_Bytes.clear();
			return;
		}
	}


	Image::Image(Image&& other) noexcept :
		m_Path{std::move(other.m_Path)},
		m_Width{other.m_Width},
		m_Height{other.m_Height},
		m_Channels{other.Channels()},
		m_Bytes{std::move(other.m_Bytes)}
	{
		other.m_Width = 0;
		other.m_Height = 0;
		other.m_Channels = 0;
	}


	auto Image::operator=(Image&& other) noexcept -> Image&
	{
		m_Path = std::move(other.m_Path);
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_Channels = other.m_Channels;
		m_Bytes = std::move(other.m_Bytes);

		other.m_Width = 0;
		other.m_Height = 0;
		other.m_Channels = 0;

		return *this;
	}


	auto Image::Path() const noexcept -> std::filesystem::path const&
	{
		return m_Path;
	}


	auto Image::Width() const noexcept -> int
	{
		return m_Width;
	}


	auto Image::Height() const noexcept -> int
	{
		return m_Height;
	}


	auto Image::Channels() const noexcept -> int
	{
		return m_Channels;
	}


	auto Image::ExtractChannel(int const channel) -> Image
	{
		if (channel < 0)
		{
			throw std::invalid_argument{"Invalid channel index \"" + std::to_string(channel) + "\". Channel index may not be negative."};
		}

		if (channel >= m_Channels)
		{
			throw std::invalid_argument{"Invalid channel index \"" + std::to_string(channel) + "\". Number of channels in image: " + std::to_string(m_Channels) + "."};
		}

		Image img;
		img.m_Width = m_Width;
		img.m_Height = m_Height;
		img.m_Channels = 1;
		img.m_Bytes.reserve(m_Width * m_Height);

		decltype(m_Bytes) newBytes;

		//for (auto i = 0; i < m_Bytes.size(); i += m_Channels - 1)
		for (auto i = 0; i < m_Bytes.size(); i += m_Channels)
		{
			img.m_Bytes.push_back(m_Bytes[i + channel]);

			for (auto j = 0; j < m_Channels; j++)
			{
				if (j != channel)
				{
					newBytes.push_back(m_Bytes[i + j]);
				}
			}
		}

		m_Bytes = newBytes;
		m_Channels -= 1;

		return img;
	}


	auto Image::operator[](std::size_t const rowIndex) const -> unsigned char const*
	{
		return m_Bytes.data() + rowIndex * m_Width * m_Channels;
	}


	auto Image::Empty() const noexcept -> bool
	{
		return m_Path.empty() || m_Width == 0 || m_Height == 0 || m_Channels == 0 && m_Bytes.empty();
	}


	auto Image::Data() const noexcept -> std::span<unsigned char const>
	{
		return m_Bytes;
	}
}
