#include "Image.hpp"

#include "Logger.hpp"

#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <utility>


namespace leopph
{
	Image::Image(std::filesystem::path const& path, ColorEncoding const colorEncoding, ImageOrientation const imageOrientation) :
		m_Encoding{colorEncoding}
	{
		stbi_set_flip_vertically_on_load(imageOrientation == ImageOrientation::FlipVertical);
		auto const pathStr = path.string();

		int width, height, channels;
		auto* const data = stbi_load(pathStr.c_str(), &width, &height, &channels, 0);

		// Skip checking result of file read in release builds.
		#ifndef NDEBUG
		if (!data)
		{
			// Width, height and channels are already initialized to 0 to signal emptiness, only need to throw error
			internal::Logger::Instance().Error("Failed to load image at " + pathStr + ". Reverting to default image.");
			return;
		}
		#endif

		m_Bytes.reset(data);
		m_Width = static_cast<u32>(width);
		m_Height = static_cast<u32>(height);
		m_Channels = static_cast<u8>(channels);
	}



	Image::Image(u32 const width, u32 const height, u8 const channels, std::unique_ptr<u8[]> bytes, ColorEncoding const colorEncoding) :
		m_Width{width},
		m_Height{height},
		m_Channels{channels},
		m_Encoding{colorEncoding},
		m_Bytes{std::move(bytes)}
	{ }



	Image::Image(Image const& other) :
		m_Width(other.m_Width),
		m_Height(other.m_Height),
		m_Channels(other.m_Channels),
		m_Encoding{other.m_Encoding},
		m_Bytes{std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(m_Width) * m_Height * m_Channels)}
	{
		std::ranges::copy(other.m_Bytes.get(), other.m_Bytes.get() + static_cast<std::size_t>(m_Width) * m_Height * m_Channels, m_Bytes.get());
	}



	Image& Image::operator=(Image const& other)
	{
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_Channels = other.m_Channels;
		m_Encoding = other.m_Encoding;
		m_Bytes = std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(m_Width) * m_Height * m_Channels);
		std::ranges::copy(other.m_Bytes.get(), other.m_Bytes.get() + static_cast<std::size_t>(m_Width * m_Height * m_Channels), m_Bytes.get());
		return *this;
	}



	Image::Image(Image&& other) noexcept :
		m_Width{other.m_Width},
		m_Height{other.m_Height},
		m_Channels{other.m_Channels},
		m_Encoding{other.m_Encoding},
		m_Bytes{std::move(other.m_Bytes)}
	{
		other.m_Width = 0;
		other.m_Height = 0;
		other.m_Channels = 0;
	}



	Image& Image::operator=(Image&& other) noexcept
	{
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_Channels = other.m_Channels;
		m_Encoding = other.m_Encoding;
		m_Bytes = std::move(other.m_Bytes);

		other.m_Width = 0;
		other.m_Height = 0;
		other.m_Channels = 0;

		return *this;
	}



	u32 Image::Width() const noexcept
	{
		return m_Width;
	}



	u32 Image::Height() const noexcept
	{
		return m_Height;
	}



	u8 Image::Channels() const noexcept
	{
		return m_Channels;
	}



	ColorEncoding Image::Encoding() const noexcept
	{
		return m_Encoding;
	}



	Image Image::ExtractChannel(u8 const channel)
	{
		// Skip bounds check in release builds.
		#ifndef NDEBUG
		if (channel >= m_Channels)
		{
			throw std::invalid_argument{"Invalid channel index \"" + std::to_string(channel) + "\". Number of channels in image is " + std::to_string(m_Channels) + "."};
		}
		#endif

		Image img;
		img.m_Width = m_Width;
		img.m_Height = m_Height;
		img.m_Channels = 1;
		img.m_Bytes = std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(m_Width) * m_Height);

		auto newBytes = std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(m_Width) * m_Height * m_Channels - 1);

		for (u64 pixel = 0, extract = 0, remaining = 0; pixel < static_cast<u64>(m_Width) * m_Height * m_Channels; pixel += m_Channels, extract++)
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



	bool Image::Empty() const noexcept
	{
		return m_Width == 0
			// Skip consistency check in release builds.
			#ifndef NDEBUG
			|| m_Height == 0 || m_Channels == 0 || !m_Bytes
			#endif
			;
	}



	std::span<u8 const> Image::operator[](u64 const rowIndex) const
	{
		return {m_Bytes.get() + rowIndex * m_Width * m_Channels, static_cast<std::span<u8 const>::size_type>(m_Width * m_Channels)};
	}



	std::span<u8 const> Image::Data() const noexcept
	{
		return {m_Bytes.get(), static_cast<std::span<u8 const>::size_type>(m_Width * m_Height * m_Channels)};
	}
}
