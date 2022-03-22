#include "Image.hpp"

#include "../util/Logger.hpp"

#include <stb_image.h>
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
			internal::Logger::Instance().Error("Failed to load image at " + pathStr + ".");
		}

		m_Bytes.assign(data, data + m_Width * m_Height * m_Channels * 8);
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


	auto Image::operator[](std::size_t const rowIndex) const -> unsigned char const*
	{
		return m_Bytes.data() + rowIndex * m_Width * m_Channels * 8;
	}
}


auto std::hash<leopph::Image>::operator()(leopph::Image const& img) const noexcept -> std::size_t
{
	return hash_value(img.Path());
}
