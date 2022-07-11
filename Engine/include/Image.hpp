#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>


namespace leopph
{
	// Specifies how color value should be interpreted
	enum class ColorEncoding : u8
	{
		Linear = 0,	// The color values are in linear RGB space
		SRGB = 1,	// The color values are in non-linear sRGB color space
	};


	// Specifies how image data is read and stored from a given byte stream
	enum class ImageOrientation : u8
	{
		Original = 0,		// Data will be stored as given
		FlipVertical = 1,	// Data will be stored as if read line-by-line bottom up
	};


	class Image
	{
		public:
			// Create an empty image.
			Image() = default;

			// Parse an image from disk. 
			explicit LEOPPHAPI Image(std::filesystem::path const& path, ColorEncoding colorEncoding = ColorEncoding::Linear, ImageOrientation imageOrientation = ImageOrientation::Original);

			// Create image from raw data.
			// Size and channel values are not checked for consistency against the byte stream.
			LEOPPHAPI Image(u32 width, u32 height, u8 channels, std::unique_ptr<u8[]> bytes, ColorEncoding colorEncoding = ColorEncoding::Linear);

			[[nodiscard]] auto LEOPPHAPI Width() const noexcept -> u32;
			[[nodiscard]] auto LEOPPHAPI Height() const noexcept -> u32;
			[[nodiscard]] auto LEOPPHAPI Channels() const noexcept -> u8;
			[[nodiscard]] auto LEOPPHAPI Encoding() const noexcept -> ColorEncoding;

			// Remove the specified color channel from the image and return it as a new, single channel image.
			// Invalidates internal data pointers.
			[[nodiscard]] auto LEOPPHAPI ExtractChannel(u8 channel) -> Image;

			// Return whether any data is held, that is if width, height or channels is 0, or bytes is nullptr.
			[[nodiscard]] auto LEOPPHAPI Empty() const noexcept -> bool;

			// Return width * channels number of bytes that forms a row of the image.
			[[nodiscard]] auto LEOPPHAPI operator[](u64 rowIndex) const -> std::span<u8 const>;

			// Returns the stored bytes.
			[[nodiscard]] auto LEOPPHAPI Data() const noexcept -> std::span<u8 const>;

			LEOPPHAPI Image(Image const& other);
			auto LEOPPHAPI operator=(Image const& other) -> Image&;

			LEOPPHAPI Image(Image&& other) noexcept;
			auto LEOPPHAPI operator=(Image&& other) noexcept -> Image&;

			~Image() noexcept = default;

		private:
			u32 m_Width{0};
			u32 m_Height{0};
			u8 m_Channels{0};
			ColorEncoding m_Encoding{ColorEncoding::Linear};
			std::unique_ptr<u8[]> m_Bytes{};
	};
}
