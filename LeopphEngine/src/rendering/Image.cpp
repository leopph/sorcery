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
		m_Bytes.reset(stbi_load(pathStr.c_str(), &m_Width, &m_Height, &m_Channels, 0));

		if (!m_Bytes)
		{
			internal::Logger::Instance().Error("Failed to load image at " + pathStr + ". Reverting to default image.");
			m_Path.clear();
			m_Width = 0;
			m_Height = 0;
			m_Channels = 0;
			return;
		}
	}


	Image::Image(int const width, int const height, int const channels, std::unique_ptr<unsigned char[]> bytes) :
		m_Width{width},
		m_Height{height},
		m_Channels{channels},
		m_Bytes{std::move(bytes)}
	{ }


	Image::Image(Image const& other) :
		m_Path(other.m_Path),
		m_Width(other.m_Width),
		m_Height(other.m_Height),
		m_Channels(other.m_Channels),
		m_Bytes{std::make_unique_for_overwrite<unsigned char[]>(m_Width * m_Height * m_Channels)}
	{
		std::ranges::copy(other.m_Bytes.get(), other.m_Bytes.get() + m_Width * m_Height * m_Channels, m_Bytes.get());
	}


	auto Image::operator=(Image const& other) -> Image&
	{
		m_Path = other.m_Path;
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_Channels = other.m_Channels;
		m_Bytes = std::make_unique_for_overwrite<unsigned char[]>(m_Width * m_Height * m_Channels);
		std::ranges::copy(other.m_Bytes.get(), other.m_Bytes.get() + m_Width * m_Height * m_Channels, m_Bytes.get());
		return *this;
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
		img.m_Bytes = std::make_unique_for_overwrite<unsigned char[]>(m_Width * m_Height);

		auto newBytes = std::make_unique_for_overwrite<unsigned char[]>(m_Width * m_Height * m_Channels - 1);

		for (auto pixel = 0, extract = 0, remaining = 0; pixel < m_Width * m_Height * m_Channels; pixel += m_Channels, extract++)
		{
			img.m_Bytes[extract] = m_Bytes[pixel + channel];

			for (auto chanOffset = 0; chanOffset < m_Channels; chanOffset++)
			{
				if (chanOffset != channel)
				{
					newBytes[remaining] = m_Bytes[pixel + chanOffset];
					remaining++;
				}
			}
		}

		m_Bytes = std::move(newBytes);
		m_Channels -= 1;

		return img;
	}


	auto Image::operator[](std::size_t const rowIndex) const -> unsigned char const*
	{
		return m_Bytes.get() + rowIndex * m_Width * m_Channels;
	}


	auto Image::Empty() const noexcept -> bool
	{
		return m_Path.empty() || m_Width == 0 || m_Height == 0 || m_Channels == 0 || !m_Bytes;
	}


	auto Image::Data() const noexcept -> std::span<unsigned char const>
	{
		return std::span{m_Bytes.get(), static_cast<std::size_t>(m_Width * m_Height * m_Channels)};
	}
}
