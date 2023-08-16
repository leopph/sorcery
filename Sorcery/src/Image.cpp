#include "Image.hpp"

#include <algorithm>
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
