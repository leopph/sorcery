#include "Image.hpp"

// #include "Logger.hpp" TODO

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <utility>


namespace leopph {
	Image::Image(std::filesystem::path const& path, ImageOrientation const imageOrientation, ColorEncoding const colorEncoding) :
		mEncoding{ colorEncoding } {
		stbi_set_flip_vertically_on_load(imageOrientation == ImageOrientation::FlipVertical);
		auto const pathStr = path.string();

		int width, height, channels;
		auto* const data = stbi_load(pathStr.c_str(), &width, &height, &channels, 0);

		// Skip checking result of file read in release builds.
#ifndef NDEBUG
		if (!data) {
			// Width, height and channels are already initialized to 0 to signal emptiness, only need to throw error
			// Logger::get_instance().error("Failed to load image at " + pathStr + ". Reverting to default image."); TODO
			return;
		}
#endif

		mData.reset(data);
		mWidth = static_cast<u32>(width);
		mHeight = static_cast<u32>(height);
		mNumChannels = static_cast<u8>(channels);
	}



	Image::Image(u32 const width, u32 const height, u8 const channels, std::unique_ptr<u8[]> bytes, ColorEncoding const colorEncoding) :
		mWidth{ width },
		mHeight{ height },
		mNumChannels{ channels },
		mEncoding{ colorEncoding },
		mData{ std::move(bytes) } {
	}



	Image::Image(Image const& other) :
		mWidth(other.mWidth),
		mHeight(other.mHeight),
		mNumChannels(other.mNumChannels),
		mEncoding{ other.mEncoding },
		mData{ std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(mWidth) * mHeight * mNumChannels) } {
		std::ranges::copy(other.mData.get(), other.mData.get() + static_cast<std::size_t>(mWidth) * mHeight * mNumChannels, mData.get());
	}



	Image& Image::operator=(Image const& other) {
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mNumChannels = other.mNumChannels;
		mEncoding = other.mEncoding;
		mData = std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(mWidth) * mHeight * mNumChannels);
		std::ranges::copy(other.mData.get(), other.mData.get() + static_cast<std::size_t>(mWidth * mHeight * mNumChannels), mData.get());
		return *this;
	}



	Image::Image(Image&& other) noexcept :
		mWidth{ other.mWidth },
		mHeight{ other.mHeight },
		mNumChannels{ other.mNumChannels },
		mEncoding{ other.mEncoding },
		mData{ std::move(other.mData) } {
		other.mWidth = 0;
		other.mHeight = 0;
		other.mNumChannels = 0;
	}



	Image& Image::operator=(Image&& other) noexcept {
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mNumChannels = other.mNumChannels;
		mEncoding = other.mEncoding;
		mData = std::move(other.mData);

		other.mWidth = 0;
		other.mHeight = 0;
		other.mNumChannels = 0;

		return *this;
	}



	u32 Image::get_width() const {
		return mWidth;
	}



	u32 Image::get_height() const {
		return mHeight;
	}



	u8 Image::get_num_channels() const {
		return mNumChannels;
	}



	ColorEncoding Image::get_encoding() const {
		return mEncoding;
	}



	void Image::set_encoding(ColorEncoding const encoding) {
		mEncoding = encoding;
	}



	Image Image::extract_channel(u8 const channel) {
		// Skip bounds check in release builds.
#ifndef NDEBUG
		if (channel >= mNumChannels) {
			throw std::invalid_argument{ "Invalid channel index \"" + std::to_string(channel) + "\". Number of channels in image is " + std::to_string(mNumChannels) + "." };
		}
#endif

		Image img;
		img.mWidth = mWidth;
		img.mHeight = mHeight;
		img.mNumChannels = 1;
		img.mData = std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(mWidth) * mHeight);

		auto newBytes = std::make_unique_for_overwrite<u8[]>(static_cast<std::size_t>(mWidth) * mHeight * mNumChannels - 1);

		for (u64 pixel = 0, extract = 0, remaining = 0; pixel < static_cast<u64>(mWidth) * mHeight * mNumChannels; pixel += mNumChannels, extract++) {
			img.mData[extract] = mData[pixel + channel];

			for (auto chanOffset = 0; chanOffset < mNumChannels; chanOffset++) {
				if (chanOffset != channel) {
					newBytes[remaining] = mData[pixel + chanOffset];
					remaining++;
				}
			}
		}

		mData = std::move(newBytes);
		mNumChannels -= 1;

		return img;
	}



	bool Image::is_empty() const {
		return mWidth == 0
			// Skip consistency check in release builds.
#ifndef NDEBUG
			|| mHeight == 0 || mNumChannels == 0 || !mData
#endif
			;
	}



	std::span<u8 const> Image::operator[](u64 const rowIndex) const {
		return { mData.get() + rowIndex * mWidth * mNumChannels, static_cast<std::span<u8 const>::size_type>(mWidth * mNumChannels) };
	}



	std::span<u8 const> Image::get_data() const {
		return { mData.get(), static_cast<std::span<u8 const>::size_type>(mWidth * mHeight * mNumChannels) };
	}
}
