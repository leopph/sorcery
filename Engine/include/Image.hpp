#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <filesystem>
#include <memory>
#include <span>


namespace leopph
{
	// Specifies how color value should be interpreted
	enum class ColorEncoding : u8
	{
		Linear = 0,	// The color values are in linear RGB space
		sRGB = 1,	// The color values are in non-linear sRGB color space
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
			explicit LEOPPHAPI Image(std::filesystem::path const& path, ImageOrientation imageOrientation = ImageOrientation::Original, ColorEncoding colorEncoding = ColorEncoding::Linear);

			// Create image from raw data.
			// Size and channel values are not checked for consistency against the byte stream.
			LEOPPHAPI Image(u32 width, u32 height, u8 channels, std::unique_ptr<u8[]> bytes, ColorEncoding colorEncoding = ColorEncoding::Linear);


			[[nodiscard]] LEOPPHAPI u32 get_width() const;
			[[nodiscard]] LEOPPHAPI u32 get_height() const;
			[[nodiscard]] LEOPPHAPI u8 get_num_channels() const;

			[[nodiscard]] LEOPPHAPI ColorEncoding get_encoding() const;
			LEOPPHAPI void set_encoding(ColorEncoding encoding);


			// Remove the specified color channel from the image and return it as a new, single channel image.
			// Invalidates internal data pointers.
			[[nodiscard]] LEOPPHAPI Image extract_channel(u8 channel);


			// Return whether any data is held, that is if width, height or channels is 0, or bytes is nullptr.
			[[nodiscard]] LEOPPHAPI bool is_empty() const;


			// Return width * channels number of bytes that forms a row of the image.
			[[nodiscard]] LEOPPHAPI std::span<u8 const> operator[](u64 rowIndex) const;


			// Returns the stored bytes.
			[[nodiscard]] LEOPPHAPI std::span<u8 const> get_data() const;


			LEOPPHAPI Image(Image const& other);
			LEOPPHAPI Image& operator=(Image const& other);

			LEOPPHAPI Image(Image&& other) noexcept;
			LEOPPHAPI Image& operator=(Image&& other) noexcept;

			~Image() = default;


		private:
			u32 mWidth{0};
			u32 mHeight{0};
			u8 mNumChannels{0};
			ColorEncoding mEncoding{ColorEncoding::Linear};
			std::unique_ptr<u8[]> mData{};
	};
}
