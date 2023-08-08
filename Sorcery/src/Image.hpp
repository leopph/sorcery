#pragma once

#include "Core.hpp"

#include <cstdint>
#include <memory>
#include <span>


namespace sorcery {
class Image {
public:
  Image() noexcept = default;
  LEOPPHAPI Image(int width, int height, int channelCount, std::unique_ptr<std::uint8_t[]> bytes) noexcept;
  LEOPPHAPI Image(Image const& other);
  LEOPPHAPI Image(Image&& other) noexcept;

  ~Image() noexcept = default;

  LEOPPHAPI auto operator=(Image const& other) -> Image&;
  LEOPPHAPI auto operator=(Image&& other) noexcept -> Image&;

  [[nodiscard]] LEOPPHAPI auto GetWidth() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetHeight() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetChannelCount() const noexcept -> int;

  // Removes the specified color channel from the image and returns it as a new, single channel image.
  [[nodiscard]] LEOPPHAPI auto ExtractChannel(int channelIdx) -> Image;
  // Appends a new channel to the image with the specified constant value.
  LEOPPHAPI auto AppendChannel(std::uint8_t value) noexcept -> void;

  // Returns whether any data is held, that is if width, height or channels is 0, or bytes is nullptr.
  [[nodiscard]] LEOPPHAPI auto IsEmpty() const noexcept -> bool;

  // Returns Width * ChannelCount bytes that form a row of the image.
  [[nodiscard]] LEOPPHAPI auto GetRow(int rowIdx) const noexcept -> std::span<std::uint8_t const>;

  // Returns the stored bytes.
  [[nodiscard]] LEOPPHAPI auto GetData() const noexcept -> std::span<std::uint8_t const>;

private:
  std::unique_ptr<std::uint8_t[]> mData{nullptr};
  int mWidth{0};
  int mHeight{0};
  int mChannelCount{0};
};
}
