#include "Image.hpp"

#include <algorithm>
#include <stb_dxt.h>

#include <array>
#include <utility>


namespace sorcery {
Image::Image(int const width, int const height, int const channelCount, std::unique_ptr<std::uint8_t[]> bytes, bool const isSrgb) noexcept :
  mData{std::move(bytes)},
  mWidth{width},
  mHeight{height},
  mChannelCount{channelCount},
  mIsSrgb{isSrgb} { }


Image::Image(Image const& other) :
  mData{std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(other.mWidth) * other.mHeight * other.mChannelCount)},
  mWidth(other.mWidth),
  mHeight(other.mHeight),
  mChannelCount(other.mChannelCount) {
  std::ranges::copy_n(other.mData.get(), static_cast<ptrdiff_t>(mWidth) * mHeight * mChannelCount, mData.get());
}


Image::Image(Image&& other) noexcept :
  mData{std::move(other.mData)},
  mWidth{other.mWidth},
  mHeight{other.mHeight},
  mChannelCount{other.mChannelCount} {
  other.mWidth = 0;
  other.mHeight = 0;
  other.mChannelCount = 0;
}


auto Image::operator=(Image const& other) -> Image& {
  mData = std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(other.mWidth) * other.mHeight * other.mChannelCount);
  mWidth = other.mWidth;
  mHeight = other.mHeight;
  mChannelCount = other.mChannelCount;
  std::ranges::copy_n(other.mData.get(), static_cast<std::ptrdiff_t>(mWidth) * mHeight * mChannelCount, mData.get());
  return *this;
}


auto Image::operator=(Image&& other) noexcept -> Image& {
  mData = std::move(other.mData);
  mWidth = other.mWidth;
  mHeight = other.mHeight;
  mChannelCount = other.mChannelCount;

  other.mWidth = 0;
  other.mHeight = 0;
  other.mChannelCount = 0;

  return *this;
}


auto Image::GetWidth() const noexcept -> int {
  return mWidth;
}


auto Image::GetHeight() const noexcept -> int {
  return mHeight;
}


auto Image::GetChannelCount() const noexcept -> int {
  return mChannelCount;
}


auto Image::IsSrgb() const noexcept -> bool {
  return mIsSrgb;
}


auto Image::SetSrgb(bool const srgb) noexcept -> void {
  mIsSrgb = srgb;
}


auto Image::ExtractChannel(int const channelIdx) -> Image {
  if (channelIdx >= mChannelCount) {
    return {};
  }

  Image ret;
  ret.mWidth = mWidth;
  ret.mHeight = mHeight;
  ret.mChannelCount = 1;
  ret.mData = std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(mWidth) * mHeight);

  auto thisNewData{std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(mWidth) * mHeight * (mChannelCount - 1))};

  for (auto pixel{0}, extract{0}, remaining{0}; pixel < mWidth * mHeight * mChannelCount; pixel += mChannelCount, extract++) {
    ret.mData[extract] = mData[pixel + channelIdx];

    for (auto chanOffset = 0; chanOffset < mChannelCount; chanOffset++) {
      if (chanOffset != channelIdx) {
        thisNewData[remaining] = mData[pixel + chanOffset];
        remaining++;
      }
    }
  }

  mData = std::move(thisNewData);
  mChannelCount -= 1;

  return ret;
}


auto Image::AppendChannel(std::uint8_t const value) noexcept -> void {
  auto const newChannelCount{mChannelCount + 1};
  auto const pixelCount{mWidth * mHeight};

  auto newData{std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(mWidth) * mHeight * newChannelCount)};

  for (auto i{0}; i < pixelCount; i++) {
    for (auto j{0}; j < mChannelCount; j++) {
      newData[i * newChannelCount + j] = mData[i * mChannelCount + j];
    }
    newData[i * newChannelCount + mChannelCount] = value;
  }

  mData = std::move(newData);
  mChannelCount += 1;
}


auto Image::CreateBlockCompressedData() const noexcept -> std::optional<BlockCompressedData> {
  if (IsEmpty() || mWidth % 4 != 0 || mHeight % 4 != 0) {
    return std::nullopt;
  }

  auto const extractBlock{
    []<std::size_t ChannelCount>(Image const& img, int const rowIdx, int const colIdx, std::array<std::uint8_t, 16 * ChannelCount>& blockData) {
      for (std::size_t k{0}; k < 4; k++) {
        std::ranges::copy_n(&img.mData[(rowIdx * img.mWidth + colIdx) * ChannelCount], 4 * ChannelCount, &blockData[k * 4 * ChannelCount]);
      }
    }
  };

  if (mChannelCount == 1) {
    BlockCompressedData ret{
      .bytes = std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(mWidth) * mHeight / 2),
      .rowByteCount = mWidth * 2
    };

    auto dstBlock{ret.bytes.get()};

    for (auto i{0}; i < mHeight; i += 4) {
      for (auto j{0}; j < mWidth; j += 4) {
        std::array<std::uint8_t, 16> blockData;
        extractBlock.operator()<1>(*this, i, j, blockData);
        stb_compress_bc4_block(dstBlock, blockData.data());
        dstBlock += 8;
      }
    }

    return ret;
  }

  if (mChannelCount == 2) {
    // TODO
  }

  if (mChannelCount == 3) {
    BlockCompressedData ret{
      .bytes = std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(mWidth) * mHeight / 2),
      .rowByteCount = mWidth * 2
    };

    auto dstBlock{ret.bytes.get()};

    Image alphaExtended{*this};
    alphaExtended.AppendChannel(255);

    for (int i{0}; i < mHeight; i += 4) {
      for (int j{0}; j < mWidth; j += 4) {
        std::array<std::uint8_t, 64> blockData;
        extractBlock.operator()<4>(alphaExtended, i, j, blockData);
        stb_compress_dxt_block(dstBlock, blockData.data(), 0, STB_DXT_HIGHQUAL);
        dstBlock += 8;
      }
    }

    return ret;
  }

  if (mChannelCount == 4) {
    BlockCompressedData ret{
      .bytes = std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(mWidth) * mHeight),
      .rowByteCount = mWidth * 4
    };

    auto dstBlock{ret.bytes.get()};

    for (int i{0}; i < mHeight; i += 4) {
      for (int j{0}; j < mWidth; j += 4) {
        std::array<std::uint8_t, 64> blockData;
        extractBlock.operator()<4>(*this, i, j, blockData);
        stb_compress_dxt_block(dstBlock, blockData.data(), 10, STB_DXT_HIGHQUAL);
        dstBlock += 16;
      }
    }

    return ret;
  }

  return std::nullopt;
}


auto Image::IsEmpty() const noexcept -> bool {
  return !mData;
}


auto Image::GetRow(int const rowIdx) const noexcept -> std::span<std::uint8_t const> {
  if (IsEmpty()) {
    return {};
  }
  return {mData.get() + static_cast<std::ptrdiff_t>(rowIdx) * mWidth * mChannelCount, static_cast<std::size_t>(mWidth * mChannelCount)};
}


auto Image::GetData() const noexcept -> std::span<std::uint8_t const> {
  if (IsEmpty()) {
    return {};
  }
  return std::span{mData.get(), static_cast<std::size_t>(mWidth) * mHeight * mChannelCount};
}
}
